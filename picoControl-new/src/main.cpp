// Animatronic robot control — main.cpp
// Vehicle selection: set -DVEHICLENAME in platformio.ini build_flags
// Hardware capabilities: set feature flags in include/config.h (per vehicle block)
// Vehicle behaviour:     see src/platform_xxx.cpp

#include <Arduino.h>
#include "config.h"
#include "platform.h"
#include "PicoRelay.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Global channel array: received RF values, mapped to 0-255
//                          0    1    2  3  4  5  6  7  8  9 10 11 12 13 14 15
int channels[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//////////////////////////////////////////////////////////////////////////////////////////////

PicoRelay relay;

// Handshake flag: Core 1 waits for this before starting CRSF/audio
// so it never touches peripherals before Core 0 has initialised them.
volatile bool _core0SetupDone = false;

#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
void processScreen(int mode, int position);
#endif

#ifdef USE_DDSM
#include <ddsm_ctrl.h>
#include <SoftwareSerial.h>
DDSM_CTRL dc;
SoftwareSerial DDSMport(18, 19);
#endif

#ifdef USE_STS
#include <SCServo.h>
#include <SoftwareSerial.h>
SMS_STS st;
SoftwareSerial STSport(13, 12);  // BOARD V3.0
#endif

#ifdef USB_JOYSTICK
#include <Joystick.h>
#endif

#ifdef USE_ENCODER
#include <hardware/pio.h>
#include "quadrature.pio.h"
#ifdef BOARD_V1
  #define QUADRATURE_A_PIN 20
  #define QUADRATURE_B_PIN 21
  #define PUSH_BUTTON 22
#else
  #define QUADRATURE_A_PIN 13
  #define QUADRATURE_B_PIN 14
  #define PUSH_BUTTON 15
#endif
PIO pio = pio0;
unsigned int sm = pio_claim_unused_sm(pio, true);
#endif

#ifdef USE_RS485
#include "RS485Reader.h"
#define FAST_SPEED 200
#define SLOW_SPEED 150
#endif

#ifdef ROBOTIS
#include <Dynamixel2Arduino.h>
#define DXL_SERIAL   Serial1
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = 2;
const uint8_t DXL_ID = 1;
const float DXL_PROTOCOL_VERSION = 2.0;
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
using namespace ControlTableItem;
#endif

#ifdef USE_M5_SERVOS
#include "M5Unit8Servos.h"
M5Unit8Servos servos;
#endif

#ifdef CAN_DRIVER
#include "mcp_can.h"
#include "TMotor_ServoConnection.h"
#define CAN0_INT 2
#endif

#ifdef CUBEMARS
#define ID_MOTOR_1 104
MCP_CAN CAN0(&SPI1, 17);
TMotor_ServoConnection servo_conn(CAN0);
#endif

#define IDLE     0
#define ACTIVE   1
#define PLAYBACK 2

#include "Action.h"

#ifdef ANIMATION_KEY
#include "Animation.h"
Animation animation(defaultAnimation, DEFAULT_STEPS);
  // Animation animation(expoAnimation, EXPO_STEPS); // animal love uses EXPO steps as default!!
#ifdef EXPO_KEY
Animation expanimation(expoAnimation, EXPO_STEPS);
#endif
// Each platform that uses recorded animations defines ANIMATION_TRACK_H
// to the name of its track header file (set in config.h per vehicle block)
#ifdef ANIMATION_TRACK_H
#pragma message("Animation track: " ANIMATION_TRACK_H)
#include ANIMATION_TRACK_H
#endif
#endif // ANIMATION_KEY

#ifdef ANIMATION_KEY
  #define ANIM_PLAYING() animation.isPlaying()
#else
  #define ANIM_PLAYING() false
#endif

#if USE_AUDIO >= 1
DFRobot_DF1201S player1;
SoftwareSerial player1port(7, 6);
#endif
#if USE_AUDIO >= 2
DFRobot_DF1201S player2;
SoftwareSerial player2port(17, 16);
#endif

// ----------------------------------------------------------------------------
// getRemoteSwitch — new protocol using crsf_channels.h
// mux_channel 0-15: reads switch banks in channels[11-14]
// getRemoteKey(bit): reads keypad bitmask in channels[9-10]
// ----------------------------------------------------------------------------
#include "crsf_channels.h"

// getRemoteSwitch  — mux channel HIGH (state=2)
// getRemoteSwitchMid — mux channel MID (state=1, centre position)
// getRemoteSwitchLow — mux channel LOW (state=0, default/off)
// Use MUX_HIGH/MUX_MID/MUX_LOW macros in action lists.
bool getRemoteSwitch(int mux_channel) {
    uint8_t sw[4];
    SWITCH_BUILD(sw, channels);
    return SWITCH_HIGH(sw, mux_channel);   // state == 2
}

bool getRemoteSwitchMid(int mux_channel) {
    uint8_t sw[4];
    SWITCH_BUILD(sw, channels);
    return SWITCH_MID(sw, mux_channel);    // state == 1
}

bool getRemoteSwitchLow(int mux_channel) {
    uint8_t sw[4];
    SWITCH_BUILD(sw, channels);
    return SWITCH_LOW(sw, mux_channel);    // state == 0
}

bool getRemoteKey(int bit) {
    return KEYPAD_PRESSED(channels[CRSF_CH_KEYPAD_LO],
                          channels[CRSF_CH_KEYPAD_HI], bit);
}

// ANIMATION_KEY_PRESSED — checks the animation key at the configured state.
// ANIMATION_KEY_STATE: 0=low, 1=mid, 2=high (default). Set in config.h.
#ifdef ANIMATION_KEY
  #ifndef ANIMATION_KEY_STATE
    #define ANIMATION_KEY_STATE 2
  #endif
  #if ANIMATION_KEY_STATE == 0
    #define ANIMATION_KEY_PRESSED getRemoteSwitchLow(ANIMATION_KEY)
  #elif ANIMATION_KEY_STATE == 1
    #define ANIMATION_KEY_PRESSED getRemoteSwitchMid(ANIMATION_KEY)
  #else
    #define ANIMATION_KEY_PRESSED getRemoteSwitch(ANIMATION_KEY)
  #endif
#endif

#ifdef USE_CRSF
#include "core1_crsf.h"
#else
#include "Radio.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
    Serial.begin(115200);
    delay(200);  // give serial monitor time to connect
    Serial.println(F("=== Animatronic Control ==="));
#ifdef ANIMATION_TRACK_H
    Serial.println(F("Track: " ANIMATION_TRACK_H));
#endif
    Serial.println(F("=========================="));
    pinMode(LED_BUILTIN, OUTPUT);

#ifdef USE_DDSM
    DDSMport.begin(115200);
    dc.pSerial = &DDSMport;
    dc.set_ddsm_type(210);
    dc.clear_ddsm_buffer();
    dc.ddsm_change_mode(4, 2);
#endif

#ifdef USE_STS
    STSport.begin(1000000);
    st.pSerial = &STSport;
#endif

    // Audio is initialised on Core 1 via platformSetup1() to keep
    // SoftwareSerial bit-banging off Core 0 (avoids UART corruption at 420kbaud)

#ifdef USE_OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
#endif

#ifdef USE_ENCODER
    pinMode(PUSH_BUTTON, INPUT_PULLUP);
    pinMode(QUADRATURE_A_PIN, INPUT_PULLUP);
    pinMode(QUADRATURE_B_PIN, INPUT_PULLUP);
    unsigned int offset = pio_add_program(pio, &quadratureA_program);
    quadratureA_program_init(pio, sm, offset, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
#endif

#if defined(BOARD_V1) || defined(BOARD_V2)
    relay.begin();
#endif

#ifdef USE_MOTOR
    configureMotors();
#endif

#ifdef USE_RS485
    RS485Init(RS485_BAUD, RS485_SR);
#endif

#ifdef ROBOTIS
    Serial1.setTX(0);
    Serial1.setRX(1);
    dxl.begin(1000000);
    dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
    dxl.ping(DXL_ID);
    dxl.torqueOff(DXL_ID);
    dxl.setOperatingMode(DXL_ID, OP_POSITION);
    dxl.torqueOn(DXL_ID);
    dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID, 0);
#endif

#ifdef CAN_DRIVER
    while (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) != CAN_OK) {
        Serial.println("Error Initializing MCP2515...");
        delay(100);
    }
    Serial.println("MCP2515 Initialized Successfully!");
    CAN0.setMode(MCP_NORMAL);
#endif

#ifdef CUBEMARS
    servo_conn.set_origin(ID_MOTOR_1, 0);
#endif

#ifdef USE_CRSF
    // CRSF is initialised on Core 1 in setup1()
#else
    RFinit();
    RFsetSettings(2);
#endif

    for (int n = 0; n < NUM_CHANNELS; n++) channels[n] = saveValues[n];

#ifdef EXPO_KEY
    pinMode(EXPO_KEY, INPUT_PULLUP);
#endif

#ifdef USE_M5_SERVOS
    delay(500);
    servos.begin(Wire, 0x25);
#endif

    // --- vehicle-specific setup (sequences, RS485 motor init, etc.) ---
    platformSetup();

    // Signal Core 1 that all hardware is initialised and it is safe to start
    _core0SetupDone = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////////////////////////////////////////////////
bool brakeState = 1;           // file-scope so platform files can read it
unsigned long brakeTimer = 0;  // file-scope so platform files can reset it

void loop() {
    static int mode = IDLE;
    static bool startedUp = false;

#ifdef RELAY_TEST
    // Serial command handler — type a command in serial monitor and press Enter:
    //   't' = run relay test (flash all 16 relays one by one)
    //   'r' = read back I2C status of relay board
    if (Serial.available()) {
        char cmd = Serial.read();
        while (Serial.available()) Serial.read();  // flush rest of line
        if (cmd == 't') {
            Serial.println("=== RELAY TEST ===");
            relay.relayTest();
            Serial.println("=== RELAY TEST DONE ===");
        } else if (cmd == 'r') {
            relay.relayStatus();
        }
    }
#endif

    // Poll functions that run outside the 20Hz gate
#ifndef USE_CRSF
    RadioPoll();
#endif
#ifdef USE_RS485
    RS485Poll();
#endif
#ifdef USE_ENCODER
    pio_sm_exec_wait_blocking(pio, sm, pio_encode_in(pio_x, 32));
    int position = pio_sm_get_blocking(pio, sm);
#else
    int position = 0;
#endif

    // -------------------------------------------------------------------------
    // 20 Hz main loop
    // -------------------------------------------------------------------------
    static unsigned long looptime;
    if (millis() < looptime + 49) return;
    looptime = millis();

    // --- CRSF: read channels mapped by Core 1 ---
#ifdef USE_CRSF
    if (crsfReady()) {
        if (digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN, LOW);
        else                          digitalWrite(LED_BUILTIN, HIGH);
        int fresh[16];
        crsfGetChannels(fresh, 16);
#if defined(ANIMATION_KEY) && defined(EXPO_KEY)
        if (!animation.isPlaying() && !expanimation.isPlaying())
#endif
        {
            for (int n = 0; n < NUM_CHANNELS; n++) channels[n] = fresh[n];
        }
        // Always update nunchuck and animation key channel regardless of playback state
        channels[8] = fresh[8];  // CRSF_CH_NUNCHUCK_BTN
#ifdef ANIMATION_KEY
        channels[11 + (ANIMATION_KEY / 4)] = fresh[11 + (ANIMATION_KEY / 4)];
#endif
        if (!startedUp) startedUp = true;
    }
#endif

    // --- Specialist hardware output ---
#ifdef USE_DDSM
    dc.ddsm_ctrl(4, map(channels[1], 0, 255, -2100, 2100), 0);
#endif

#ifdef USE_STS
    st.WritePosEx(16, map(channels[0], 0, 255, 1024, 3072), 2000, 100);
    st.WritePosEx(20, map(channels[5], 0, 255, 0, 1048), map(channels[6], 0, 255, 0, 2048), 100);
    st.WritePosEx(15, map(channels[1], 0, 255, 512, 2048), 2000, 100);
    st.WritePosEx(14, map(channels[2], 0, 255, 2700, 1024), 1000, 20);
    { int cv = 1875, val = map(channels[2], 0, 255, -625, 625), off = 300;
      st.WritePosEx(13, cv - val + off, 1000, 20);
      st.WritePosEx(12, cv + val,       1000, 20); }
    st.WritePosEx(11, map(channels[3], 0, 255, 1024, 3072), 1000, 20);
#endif

    // --- DC motor drive (cross-mix) ---
#ifdef USE_MOTOR
#if defined(ANIMATION_KEY) && defined(EXPO_KEY)
    if (!animation.isPlaying() && !expanimation.isPlaying())
#endif
    {
#ifdef USE_SPEEDSCALING
        // Speed mode via nunchuck buttons (channels[8]):
        //   C+Z (both=192) = drive high speed
        //   C only (64)    = drive normal speed
        //   C+Z both (192) = high speed
        //   C only  (192 caught by BOTH first, then 64) = normal speed
        //   Z only  (128) = eyelid mode, no drive
        //   neither (0)   = stopped
        if (NUNCHUCK_BOTH(channels)) {
            brakeTimer = BRAKE_TIMEOUT;
            motorLeft.setSpeed( getLeftValueFromCrossMix( map(channels[1], 0, 255, -HIGH_SPEED, HIGH_SPEED), map(channels[0], 255, 0, -HIGH_SPEED, HIGH_SPEED)), brakeState);
            motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -HIGH_SPEED, HIGH_SPEED), map(channels[0], 255, 0, -HIGH_SPEED, HIGH_SPEED)), brakeState);
        } else if (NUNCHUCK_C(channels)) {
            brakeTimer = BRAKE_TIMEOUT;
            motorLeft.setSpeed( getLeftValueFromCrossMix( map(channels[1], 0, 255, -LOW_SPEED, LOW_SPEED), map(channels[0], 255, 0, -LOW_SPEED, LOW_SPEED)), brakeState);
            motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -LOW_SPEED, LOW_SPEED), map(channels[0], 255, 0, -LOW_SPEED, LOW_SPEED)), brakeState);
        } else {
            motorLeft.setSpeed(0, brakeState);
            motorRight.setSpeed(0, brakeState);
        }
#else
        brakeTimer = BRAKE_TIMEOUT;
        motorLeft.setSpeed( getLeftValueFromCrossMix( map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED), map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED)), brakeState);
        motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED), map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED)), brakeState);
#endif
    }
#endif // USE_MOTOR

#ifdef INPUT_DEBUG
    {
        static unsigned long _inputDumpTimer;
        if (millis() - _inputDumpTimer > 500) {
            _inputDumpTimer = millis();
            uint8_t sw[4]; SWITCH_BUILD(sw, channels);
            Serial.print("SW[0-3]:");
            for (int i = 0; i < 16; i++) {
                Serial.print(" "); Serial.print(i); Serial.print("=");
                Serial.print(SWITCH_STATE(sw, i));
            }
            uint16_t keys = KEYPAD_BITMASK(channels[CRSF_CH_KEYPAD_LO],
                                           channels[CRSF_CH_KEYPAD_HI]);
            Serial.print("  KEYS:0x"); Serial.print(keys, HEX);
            if (keys) {
                const char kc[] = "147*2580369#";
                Serial.print(" (");
                for (int b = 0; b < 12; b++)
                    if (keys & (1<<b)) { Serial.print(kc[b]); }
                Serial.print(")");
            }
            Serial.print("  NUNCHUCK:"); Serial.println(channels[CRSF_CH_NUNCHUCK_BTN]);
        }
    }
#endif

#ifdef ANIMATION_DEBUG
    // Records in animationStep struct field order — paste directly into Track-xxx.h:
    // {xSetpoint, ySetpoint, buttons, keypad, volume, switches1, switches2, switches3, switches4}
    // buttons = nunchuck byte (0=none, 64=C, 128=Z, 192=both) — mapped from CRSF_CH_NUNCHUCK_BTN
    // keypad  = keypad lo byte (bits 0-7 of 12-key bitmask)
    {
        Serial.print('{');
        Serial.print(channels[CRSF_CH_AXIS_X1]);         Serial.print(',');
        Serial.print(channels[CRSF_CH_AXIS_Y1]);         Serial.print(',');
        Serial.print(channels[CRSF_CH_NUNCHUCK_BTN]);   Serial.print(',');  // buttons (speed/nunchuck)
        Serial.print(channels[CRSF_CH_KEYPAD_LO]);      Serial.print(',');  // keypad lo
        Serial.print(channels[CRSF_CH_ANALOG1]);         Serial.print(',');  // volume
        Serial.print(channels[CRSF_CH_SW_MUX_0_3]);     Serial.print(',');  // switches1
        Serial.print(channels[CRSF_CH_SW_MUX_4_7]);     Serial.print(',');  // switches2
        Serial.print(channels[CRSF_CH_SW_MUX_8_11]);    Serial.print(',');  // switches3
        Serial.print(channels[CRSF_CH_SW_MUX_12_15]);                       // switches4
        Serial.println("},");
    }
#endif

    // --- Animation playback (recorded tracks) ---
    // DIRECT: plays while switch is held, loops. Stops on release.
#ifdef ANIMATION_KEY
#ifdef ANIMATION_KEY_DEBUG
    {   // Dump switch banks to identify which mux channel the anim switch is on
        static unsigned long _swDumpTimer;
        if (millis() - _swDumpTimer > 1000) {
            _swDumpTimer = millis();
            uint8_t sw[4]; SWITCH_BUILD(sw, channels);
            Serial.print("SW banks: ");
            for (int i = 0; i < 4; i++) { Serial.print(sw[i], BIN); Serial.print(" "); }
            Serial.print(" animKey(mux"); Serial.print(ANIMATION_KEY);
            Serial.print(")="); Serial.println(ANIMATION_KEY_PRESSED);
        }
    }
#endif
    if (ANIMATION_KEY_PRESSED && !animation.isPlaying()) {
        animation.start();
#ifdef USE_MOTOR
        motorLeft.setSpeed(0, brakeState);
        motorRight.setSpeed(0, brakeState);
#endif
    }
    if (animation.isPlaying() && !ANIMATION_KEY_PRESSED) animation.stop();
#endif

#ifdef EXPO_KEY
    if (mode == IDLE) {
        if (!digitalRead(EXPO_KEY) && !expanimation.isPlaying()) expanimation.start();
        if (expanimation.isPlaying() && digitalRead(EXPO_KEY)) {
            expanimation.stop();
            motorLeft.setSpeed(0, brakeState);
            motorRight.setSpeed(0, brakeState);
            for (int n = 0; n < NUM_CHANNELS; n++) channels[n] = saveValues[n];
        }
        if (expanimation.isPaused() || !expanimation.isPlaying()) {
            motorLeft.setSpeed(0, brakeState);
            motorRight.setSpeed(0, brakeState);
        } else {
            motorLeft.setSpeed(-40, brakeState);
            motorRight.setSpeed(-40, brakeState);
        }
    }
#endif

    // --- Recorded animation update ---
    // Runs BEFORE RS485 so animation channel values are transmitted when playing
#ifdef ANIMATION_KEY
    animation.update();
#endif
#ifdef EXPO_KEY
    expanimation.update();
#endif

    // --- RS485 passthrough or vehicle steering ---
#ifdef USE_RS485
#ifdef BUFFER_PASSTHROUGH
    {
        unsigned char headMessage[BUFFER_PASSTHROUGH];
        // Assemble buffer in the layout the receiving unit expects.
        // Old protocol positions preserved so the receiver doesn't need changing:
        // [0] = X axis
        // [1] = Y axis
        // [2] = speed mode: 192=high (nunchuck C), 128=low (nunchuck Z), 0=stop
        // [3] = keypad: first pressed key as ASCII char (0 if none)
        // [4] = volume (analog1)
        // [5] = switch bank 0-7  (mux ch 0-3 packed as 2 bits each in channels[11])
        // [6] = switch bank 8-15 (mux ch 4-7 in channels[12])
        // [7] = switch bank 16-23 (mux ch 8-11 in channels[13])
        // [8] = switch bank 24-31 (mux ch 12-15 in channels[14])
        headMessage[0] = channels[CRSF_CH_AXIS_X1];
        headMessage[1] = channels[CRSF_CH_AXIS_Y1];
        // Raw nunchuck buttons — receiving unit interprets: 0=none, 64=C, 128=Z, 192=both
        headMessage[2] = channels[CRSF_CH_NUNCHUCK_BTN];
        // Keypad: find first pressed key, encode as ASCII
        {
            uint16_t keys = KEYPAD_BITMASK(channels[CRSF_CH_KEYPAD_LO],
                                           channels[CRSF_CH_KEYPAD_HI]);
            const char keyChars[] = "147*2580369#";
            headMessage[3] = 0;
            for (int b = 0; b < 12; b++) {
                if (keys & (1 << b)) { headMessage[3] = keyChars[b]; break; }
            }
        }
        headMessage[4] = channels[CRSF_CH_ANALOG1];
        headMessage[5] = channels[CRSF_CH_SW_MUX_0_3];
        headMessage[6] = channels[CRSF_CH_SW_MUX_4_7];
        headMessage[7] = channels[CRSF_CH_SW_MUX_8_11];
        headMessage[8] = channels[CRSF_CH_SW_MUX_12_15];
        RS485WriteBuffer(13, headMessage, BUFFER_PASSTHROUGH);

#ifdef RS485_DEBUG
        static unsigned long _rs485DumpTimer;
        if (millis() - _rs485DumpTimer > 1000) {
            _rs485DumpTimer = millis();
            Serial.print("RS485 TX: ");
            for (int i = 0; i < BUFFER_PASSTHROUGH; i++) {
                Serial.print(headMessage[i]);
                Serial.print(" ");
            }
            Serial.print("  nunchuck_raw=");
            Serial.print(channels[CRSF_CH_NUNCHUCK_BTN]);
            Serial.print(" [mode=");
            Serial.print(mode == ACTIVE ? "ACTV" : "IDLE");
            Serial.print(" rf=");
            Serial.print(crsfReady() ? "OK" : "NO");
            Serial.println("]");
        }
#endif
    }
#else
    // Steering and drive for RS485-connected motor controllers (AMI, SCUBA, etc.)
    if ((channels[0] > 140 || channels[0] < 100) && mode == ACTIVE && !ANIM_PLAYING())
        RS485WriteByte(22, 1, map(channels[0], 0, 255, 255, 0));
    else
        RS485WriteByte(22, 1, 127);

    if (NUNCHUCK_BOTH(channels)) {
        if      (channels[1] < 110 && mode == ACTIVE && !ANIM_PLAYING())
            RS485WriteByte(18, 3, constrain(map(channels[1], 0, 110, FAST_SPEED, 0), 0, FAST_SPEED));
        else if (channels[1] > 135 && mode == ACTIVE && !ANIM_PLAYING())
            RS485WriteByte(18, 1, constrain(map(channels[1], 135, 255, 0, FAST_SPEED), 0, FAST_SPEED));
        else
            RS485WriteByte(18, 2, 0);
    } else if (NUNCHUCK_C(channels)) {
        if      (channels[1] < 110 && mode == ACTIVE && !ANIM_PLAYING())
            RS485WriteByte(18, 3, constrain(map(channels[1], 0, 110, SLOW_SPEED, 0), 0, SLOW_SPEED));
        else if (channels[1] > 135 && mode == ACTIVE && !ANIM_PLAYING())
            RS485WriteByte(18, 1, constrain(map(channels[1], 135, 255, 0, SLOW_SPEED), 0, SLOW_SPEED));
        else
            RS485WriteByte(18, 2, 0);
    } else {
        RS485WriteByte(18, 2, 0);
    }
#endif // BUFFER_PASSTHROUGH
#endif // USE_RS485

#ifdef ROBOTIS
    dxl.setGoalPosition(DXL_ID, map(channels[2], 0, 255, 1024, 3072));
#endif

#ifdef CUBEMARS
    servo_conn.set_pos_spd(ID_MOTOR_1, 180, 1000, 1000);
    if (!digitalRead(CAN0_INT)) {
        while (CAN_MSGAVAIL == CAN0.checkReceive()) servo_conn.can_receive();
    }
    servo_conn.print_motor_vars(ID_MOTOR_1);
#endif

    // --- Timeout / mode management ---
#ifdef USE_CRSF
    // Loss: crsfLost() is true when Core 1 confirmed 500ms silence.
    // We cannot use crsfReady() here — it's already false when LINK_LOST.
    if (crsfLost() && mode == ACTIVE) {
#else
    if (getTimeOut() > 9 && mode == ACTIVE) {
#endif
        mode = IDLE;
        digitalWrite(LED_BUILTIN, HIGH);
#if defined(ANIMATION_KEY) && defined(EXPO_KEY)
        if (!animation.isPlaying() && !expanimation.isPlaying())
#endif
        {
            for (int n = 0; n < NUM_CHANNELS; n++) {
#ifdef ANIMATION_KEY
                if (n == 8) continue;  // preserve animation key channel
#endif
                channels[n] = saveValues[n];
            }
        }
        platformOnIdle();
    }
#ifdef USE_CRSF
    // Recovery: crsfReady() becomes true again once Core 1 receives a good frame
    else if (crsfReady() && mode == IDLE) {
#else
    else if (getTimeOut() < 1 && mode == IDLE) {
#endif
        if (startedUp) mode = ACTIVE;
    }

    // --- Actions and vehicle-specific loop ---
#ifdef NUM_ACTIONS
    for (int n = 0; n < NUM_ACTIONS; n++) myActionList[n].update();
#endif
    platformLoop();   // sequences, background audio, vehicle-specific I/O

    // Timeout nudge (non-CRSF radio only — CRSF uses real-time millis on Core 1)
#ifndef USE_CRSF
    nudgeTimeOut();
#endif

    // --- Brake timer ---
#ifdef USE_MOTOR
    if (brakeTimer > 0) { brakeTimer--; brakeState = 0; }
    if (brakeTimer == 0) brakeState = 1;
#endif

    // --- Volume control ---
#if USE_AUDIO >= 1
    static int volume;
    if (channels[VOLUME_CHANNEL] != volume) {
        volume = channels[VOLUME_CHANNEL];
        player1.setVol(map(volume, 0, 255, 0, 32));
#if USE_AUDIO >= 2
        player2.setVol(map(volume, 0, 255, 0, 32));
#endif
    }
#endif

    watchdog_update();

    // -------------------------------------------------------------------------
    // End of 20Hz loop
    // -------------------------------------------------------------------------

    // Screen update on separate (slower) timer
#ifdef USE_OLED
    static unsigned long screentimer;
    if (millis() > screentimer + 99) {
        screentimer = millis();
        processScreen(mode, position);
    }
#endif

} // end loop()

//////////////////////////////////////////////////////////////////////////////////////////////
// CORE 1 — USB joystick (LUMI) and platform Core 1 tasks
//////////////////////////////////////////////////////////////////////////////////////////////
void setup1() {
    // Wait until Core 0 has finished all hardware initialisation including audioInit().
    extern volatile bool _core0SetupDone;
    while (!_core0SetupDone) tight_loop_contents();

#ifdef USE_CRSF
    crsfCore1Setup();
#endif
    platformSetup1();   // now always empty or near-empty (no blocking calls)
}

void loop1() {
#ifdef USE_CRSF
    crsfCore1Loop();    // Core 1: CRSF only — no audio, no blocking calls
#endif
    platformLoopCore1(); // empty for all vehicles — kept for future use
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_RS485
void ProcessRS485Data(int ID, int dataLength, unsigned char *Data) {}
#endif

#ifndef USE_CRSF
void ProcessRadioData(int ID, int dataLength, unsigned char *Data) {
    if (digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN, LOW);
    else                          digitalWrite(LED_BUILTIN, HIGH);
    if (Data[0] == 0x03) {
        for (int n = 0; n < NUM_CHANNELS; n++) channels[n] = Data[n + 1];
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// OLED screen
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_OLED

// ── Draw helpers (identical layout to picoRemote transmitter) ─────────────────

static void drawJoystick(int cx, int cy, int r, int xVal, int yVal, bool invertY = false) {
    display.drawCircle(cx, cy, r, SSD1306_WHITE);
    display.drawLine(cx, cy-r+1, cx, cy+r-1, SSD1306_WHITE);
    display.drawLine(cx-r+1, cy, cx+r-1, cy, SSD1306_WHITE);
    int dx = map(xVal, 0, 255, cx-r+2, cx+r-2);
    int dy = invertY ? map(yVal, 0, 255, cy+r-2, cy-r+2)
                     : map(yVal, 0, 255, cy-r+2, cy+r-2);
    display.fillCircle(dx, dy, 2, SSD1306_WHITE);
}

// 3 bars bottom-anchored at y. rssi_dbm is negative: -40=full, -100=empty.
static void drawSignalBars(int x, int y, int rssi_dbm) {
    int strength = constrain(map(rssi_dbm, -100, -40, 0, 3), 0, 3);
    int heights[3] = {3, 5, 7};
    for (int i = 0; i < 3; i++) {
        int bx = x + i * 4, bh = heights[i];
        if (i < strength) display.fillRect(bx, y-bh+1, 3, bh, SSD1306_WHITE);
        else              display.drawRect(bx, y-bh+1, 3, bh, SSD1306_WHITE);
    }
}

// Horizontal volume bar labelled "v".
static void drawVolBar(int x, int y, int val) {
    display.setCursor(x, y);
    display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
    display.print("v");
    int bx = x+7, bw = 30, bh = 4;
    display.drawRect(bx, y, bw, bh, SSD1306_WHITE);
    int fill = map(val, 0, 255, 0, bw-2);
    if (fill > 0) display.fillRect(bx+1, y+1, fill, bh-2, SSD1306_WHITE);
}

// 4×4 switch grid (16 mux channels). dot=low, square=mid, filled=high.
static void drawSwitches(int x, int y, uint8_t sw[4]) {
    for (int i = 0; i < 16; i++) {
        int state = SWITCH_STATE(sw, i);
        int px = x + (i%4)*5, py = y + (i/4)*5;
        if      (state == 2) display.fillRect(px, py, 4, 4, SSD1306_WHITE);
        else if (state == 1) display.drawRect(px, py, 4, 4, SSD1306_WHITE);
        else                 display.drawPixel(px+2, py+2, SSD1306_WHITE);
    }
}

// 3×4 keypad grid (12 keys). filled=pressed, dot=unpressed.
static void drawKeypad(int x, int y, uint16_t keyBits) {
    for (int i = 0; i < 12; i++) {
        int px = x + (i/4)*5, py = y + (i%4)*4;
        if ((keyBits >> i) & 1) display.fillRect(px, py, 4, 3, SSD1306_WHITE);
        else                    display.drawPixel(px+2, py+1, SSD1306_WHITE);
    }
}

// Animation track indicator (replaces battery from transmitter layout).
// Shows stripped track name and elapsed seconds if playing.

// ── processScreen ─────────────────────────────────────────────────────────────
//
//  Layout (128×32 px) — mirrors picoRemote transmitter, battery→animtrack:
//
//  y=0     [NAME+mode (0)] [animtrack (38)] [LQ:xx% (84)]
//  y=9-28  [J1 r=9 @(10,19)] [C/Z dots @21] [J2 r=9 @(34,19)]
//          [switches 4×4 @(44,11)] [keypad 3×4 @(65,11)]
//          [signal bars @(84,15)] [dBm text @(97,9)]
//          [vol bar @(84,24)]
//
void processScreen(int mode, int position) {
    display.clearDisplay();
#ifdef OLED_ROTATE
    display.setRotation(2);
#endif
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // ── Top row: fixed positions ──────────────────────────────────────────────
    // x=0:  name (max 6 chars)
    // x=20: act/idle (3-4 chars)
    // x=44: sq:Xs when playing (max 6 chars), blank otherwise
    // x=86: LQ:xxx% (7 chars, ends at x=128)
    display.setCursor(0, 0);
#if defined(SCUBA)
    display.print("SCU");
#elif defined(AMI)
    display.print("AMI");
#elif defined(LUMI)
    display.print("LUM");
#elif defined(ANIMALTRONIEK_KREEFT)
    display.print("KRE");
#elif defined(ANIMALTRONIEK_VIS)
    display.print("VIS");
#elif defined(ANIMALTRONIEK_SCHILDPAD)
    display.print("SCH");
#elif defined(ANIMAL_LOVE)
    display.print("LOV");
#elif defined(WASHMACHINE)
    display.print("WAS");
#else
    display.print("?");
#endif

    display.setCursor(20, 0);
    display.print(mode == ACTIVE ? "act" : "idle");

    display.setCursor(44, 0);
#ifdef ANIMATION_KEY
    if (animation.isPlaying()) {
        display.print("sq:");
        display.print(animation.elapsedSeconds());
        display.print("s");
    } else {
        platformScreen();   // e.g. "jaws" when jaws sequence is playing
    }
#else
    platformScreen();
#endif

#ifdef USE_CRSF
    CrsfLinkStats ls;
    crsfGetLinkStats(ls);
    display.setCursor(86, 0);
    display.print("LQ:");
    display.print(ls.link_quality);
    display.print("%");
#endif

    // ── Joystick 1 (x=10, y=19, r=9) — nunchuck Y inverted ──────────────────
    drawJoystick(10, 19, 9, channels[CRSF_CH_AXIS_X1], channels[CRSF_CH_AXIS_Y1], true);

    // ── Nunchuck C/Z button indicators at x=21 ───────────────────────────────
    {
        uint8_t btn = channels[CRSF_CH_NUNCHUCK_BTN];
        if (btn == 128 || btn == 192) display.fillRect(21, 13, 3, 3, SSD1306_WHITE); // Z
        if (btn == 64  || btn == 192) display.fillRect(21, 19, 3, 3, SSD1306_WHITE); // C
    }

    // ── Joystick 2 (x=34, y=19, r=9) — only draw dot when connected ──────────
    {
        int x2 = channels[CRSF_CH_AXIS_X2];
        int y2 = channels[CRSF_CH_AXIS_Y2];
        bool connected = !(x2 == 0 && y2 == 0);  // 0,0 = not transmitted
        // Draw circle + crosshair always, but only fill dot if connected
        display.drawCircle(34, 19, 9, SSD1306_WHITE);
        display.drawLine(34, 11, 34, 27, SSD1306_WHITE);
        display.drawLine(26, 19, 42, 19, SSD1306_WHITE);
        if (connected) {
            int dx = map(x2, 0, 255, 26, 42);
            int dy = map(y2, 0, 255, 11, 27);
            display.fillCircle(dx, dy, 2, SSD1306_WHITE);
        }
    }

    // ── Switches 4×4 grid at x=44, y=11 ─────────────────────────────────────
    {
        uint8_t sw[4];
        SWITCH_BUILD(sw, channels);
        drawSwitches(44, 11, sw);
    }

    // ── Keypad 3×4 grid at x=65, y=11 ───────────────────────────────────────
    {
        uint16_t keys = KEYPAD_BITMASK(channels[CRSF_CH_KEYPAD_LO],
                                       channels[CRSF_CH_KEYPAD_HI]);
        drawKeypad(65, 11, keys);
    }

    // ── Signal bars + dBm at x=84 ────────────────────────────────────────────
#ifdef USE_CRSF
    drawSignalBars(84, 15, (int)ls.rssi_ant1);
    display.setCursor(97, 9);
    display.print((int)ls.rssi_ant1);
    display.print("d");
#endif

    // ── Volume bar at x=84, y=24 ─────────────────────────────────────────────
    drawVolBar(84, 24, channels[CRSF_CH_ANALOG1]);

    display.display();
}
#endif