#include "ak60.h"

typedef enum {
    CAN_PACKET_SET_DUTY = 0,       // Duty Cycle Mode
    CAN_PACKET_SET_CURRENT,        // Current Loop Mode
    CAN_PACKET_SET_CURRENT_BRAKE,  // Current Brake Mode
    CAN_PACKET_SET_RPM,            // Speed Mode
    CAN_PACKET_SET_POS,            // Position Mode
    CAN_PACKET_SET_ORIGIN_HERE,    // Set Origin Mode
    CAN_PACKET_SET_POS_SPD,        // Position-Speed Loop Mode
} CAN_PACKET_ID;

// Feedback from the last successfully-parsed status frame
static volatile float   ak60_fb_pos  = 0.0f;
static volatile float   ak60_fb_vel  = 0.0f;
static volatile float   ak60_fb_cur  = 0.0f;
static volatile int8_t  ak60_fb_temp = 0;
static volatile uint8_t ak60_fb_err  = 0;
static volatile bool    ak60_fb_new  = false;

// CAN callback counters — updated from PIO interrupt
static volatile uint32_t ak60_tx_ok  = 0;
static volatile uint32_t ak60_errors = 0;

// Raw, unfiltered capture of the last successfully-parsed RX frame —
// populated regardless of the AK60_ID filter below, useful for diagnostics.
static volatile uint32_t ak60_raw_id      = 0;
static volatile uint8_t  ak60_raw_data[8] = {0};
static volatile bool     ak60_raw_new     = false;

static void buffer_append_int32(uint8_t *buffer, int32_t number, int32_t *index) {
    buffer[(*index)++] = number >> 24;
    buffer[(*index)++] = number >> 16;
    buffer[(*index)++] = number >> 8;
    buffer[(*index)++] = number;
}

static void buffer_append_int16(uint8_t *buffer, int16_t number, int16_t *index) {
    buffer[(*index)++] = number >> 8;
    buffer[(*index)++] = number;
}

// CAN RX/TX/ERR callback — called from PIO interrupt
// Servo-mode status packet layout (from TMotor_ServoConnection::parse_CAN_data,
// confirmed working over MCP2515):
//   data[0..1] = position (int16, x0.1)
//   data[2..3] = speed    (int16, x10)
//   data[4..5] = current  (int16, x0.01)
//   data[6]    = temperature
//   data[7]    = error code
static void canCallback(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg) {
    if (notify == CAN2040_NOTIFY_RX && msg && msg->dlc == 8) {
        // Stash raw frame unconditionally, before any filtering, for diagnostics.
        ak60_raw_id = msg->id;
        memcpy((void*)ak60_raw_data, msg->data, 8);
        ak60_raw_new = true;

        uint32_t motorId = msg->id & 0xFF;  // low byte of extended ID
        if (motorId != AK60_ID) return;     // ignore other nodes
        int16_t posInt = (int16_t)(((uint16_t)msg->data[0] << 8) | msg->data[1]);
        int16_t spdInt = (int16_t)(((uint16_t)msg->data[2] << 8) | msg->data[3]);
        int16_t curInt = (int16_t)(((uint16_t)msg->data[4] << 8) | msg->data[5]);
        ak60_fb_pos  = (float)posInt * 0.1f;
        ak60_fb_vel  = (float)spdInt * 10.0f;
        ak60_fb_cur  = (float)curInt * 0.01f;
        ak60_fb_temp = (int8_t)msg->data[6];
        ak60_fb_err  = msg->data[7];
        ak60_fb_new  = true;
    } else if (notify == CAN2040_NOTIFY_TX) {
        ak60_tx_ok++;
    } else if (notify == CAN2040_NOTIFY_ERROR) {
        ak60_errors++;
    }
}

// PIO1 — kept off PIO0 in case a platform needs that for something else
// (e.g. quadrature encoders). Boards used here are plain Pico, not Pico W,
// so no CYW43 WiFi reservation applies.
// IMPORTANT: can2040_start() internally calls start(sys_clk, bitrate, gpio_rx, gpio_tx)
// so constructor arg order is (pio, gpio_tx, gpio_rx, ...) but passed as rx/tx internally.
// We pass TX=AK60_CAN_TX, RX=AK60_CAN_RX — library swaps them correctly in can2040_start.
static ACAN2040 can2040(1, AK60_CAN_TX, AK60_CAN_RX, 1000000, F_CPU, canCallback);

static void ak60SendRaw(uint32_t packetId, uint8_t *d, uint8_t len) {
    struct can2040_msg msg;
    msg.id  = (AK60_ID | (packetId << 8)) | CAN2040_ID_EFF;  // extended frame
    msg.dlc = len;
    memcpy(msg.data, d, len);
    if (can2040.ok_to_send()) can2040.send_message(&msg);
}

// Servo-mode position+speed command — motor moves to pos (deg) with capped
// speed and acceleration.
//   spd/accel: int16, raw "electrical speed" units — used directly, matching
//   the values shown in CubeMars's own config tool (e.g. 5000/30000 default).
// NOTE: TMotor_ServoConnection.cpp's version of this call divides spd/accel
// by 10 before packing — that does NOT match CubeMars's own tool defaults,
// which are clearly raw values. Going with raw/unscaled here.
void ak60SetPosSpd(float posDeg, int16_t spd, int16_t accel) {
    int32_t idx  = 0;
    int16_t idx1 = 4;
    uint8_t buf[8];
    buffer_append_int32(buf, (int32_t)(posDeg * 10000.0f), &idx);
    buffer_append_int16(buf, spd,   &idx1);
    buffer_append_int16(buf, accel, &idx1);
    ak60SendRaw(CAN_PACKET_SET_POS_SPD, buf, 8);
}

// Plain Servo-mode position command — no speed/accel fields, motor uses its
// own default internal trajectory profile.
void ak60SetPos(float posDeg) {
    int32_t idx = 0;
    uint8_t buf[4];
    buffer_append_int32(buf, (int32_t)(posDeg * 10000.0f), &idx);
    ak60SendRaw(CAN_PACKET_SET_POS, buf, idx);
}

// Servo-mode current command — 0A gives no commanded torque, i.e. backdrivable.
void ak60SetCurrent(float amps) {
    int32_t idx = 0;
    uint8_t buf[4];
    buffer_append_int32(buf, (int32_t)(amps * 1000.0f), &idx);
    ak60SendRaw(CAN_PACKET_SET_CURRENT, buf, idx);
}

// Set origin — 0=temporary (power-fail elimination, not flash-saved),
// 1=permanent zero point (flash-saved), 2=restore factory default zero
// (flash-saved). Per manual §5.1.6.
void ak60SetOrigin(uint8_t mode) {
    uint8_t buf[1] = { mode };
    ak60SendRaw(CAN_PACKET_SET_ORIGIN_HERE, buf, 1);
}

float   ak60Pos()  { return ak60_fb_pos; }
float   ak60Vel()  { return ak60_fb_vel; }
float   ak60Cur()  { return ak60_fb_cur; }
int8_t  ak60Temp() { return ak60_fb_temp; }
uint8_t ak60Err()  { return ak60_fb_err; }

void ak60PrintStatus() {
    struct can2040_stats stats;
    can2040.get_statistics(&stats);
    Serial.println(F("--- CAN/AK60 status ---"));
    Serial.print(F("  ok_to_send: ")); Serial.println(can2040.ok_to_send() ? F("yes") : F("no"));
    Serial.print(F("  tx_total:   ")); Serial.println(stats.tx_total);
    Serial.print(F("  tx_attempt: ")); Serial.println(stats.tx_attempt);
    Serial.print(F("  rx_total:   ")); Serial.println(stats.rx_total);
    Serial.print(F("  parse_err:  ")); Serial.println(stats.parse_error);
    Serial.print(F("  fb_pos:     ")); Serial.println(ak60_fb_pos, 3);
    Serial.print(F("  fb_vel:     ")); Serial.println(ak60_fb_vel, 2);
    Serial.print(F("  fb_cur:     ")); Serial.println(ak60_fb_cur, 2);
    Serial.print(F("  fb_temp:    ")); Serial.println(ak60_fb_temp);
    Serial.print(F("  fb_err:     ")); Serial.println(ak60_fb_err);
}

static bool ak60_streamOn = false;  // off by default — enable via ak60SetDebugStream(true)

void ak60SetDebugStream(bool on) { ak60_streamOn = on; }
bool ak60DebugStream()           { return ak60_streamOn; }

void ak60DebugPrint() {
    if (!ak60_streamOn) return;

    static unsigned long lastCAN = 0;
    if (millis() - lastCAN <= 500) return;
    lastCAN = millis();

    struct can2040_stats stats;
    can2040.get_statistics(&stats);

    if (ak60_raw_new) {
        Serial.print(F("[AK60 RAW] id="));
        Serial.print(ak60_raw_id, HEX);
        Serial.print(F(" data="));
        for (int i = 0; i < 8; i++) {
            if (ak60_raw_data[i] < 0x10) Serial.print('0');
            Serial.print(ak60_raw_data[i], HEX);
            Serial.print(' ');
        }
        Serial.println();
        ak60_raw_new = false;
    }
    if (ak60_fb_new) {
        Serial.print(F("[AK60] pos=")); Serial.print(ak60_fb_pos, 3);
        Serial.print(F(" vel=")); Serial.print(ak60_fb_vel, 1);
        Serial.print(F(" cur=")); Serial.print(ak60_fb_cur, 2);
        Serial.print(F(" tmp=")); Serial.print(ak60_fb_temp);
        if (ak60_fb_err) { Serial.print(F(" ERR=")); Serial.print(ak60_fb_err); }
        Serial.print(F(" tx=")); Serial.print(stats.tx_total);
        Serial.print(F(" rx=")); Serial.println(stats.rx_total);
        ak60_fb_new = false;
    } else {
        Serial.print(F("[AK60] NO FB tx=")); Serial.print(stats.tx_total);
        Serial.print(F(" att=")); Serial.print(stats.tx_attempt);
        Serial.print(F(" rx=")); Serial.print(stats.rx_total);
        Serial.print(F(" err=")); Serial.println(stats.parse_error);
    }
}

static bool ak60_inited = false;

void ak60Begin() {
    if (ak60_inited) return;
    Serial.println(F("[CAN] begin() on PIO1..."));
    can2040.begin();
    Serial.println(F("[CAN] begin() returned"));
    delay(100);
    ak60PrintStatus();
    ak60_inited = true;
}

bool ak60Ready() { return ak60_inited; }

uint32_t ak60TxTotal() {
    struct can2040_stats stats;
    can2040.get_statistics(&stats);
    return stats.tx_total;
}