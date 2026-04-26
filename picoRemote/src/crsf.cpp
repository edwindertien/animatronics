// crsf.cpp — bidirectional CRSF driver
//
// TX: sends RC_CHANNELS_PACKED at the configured rate (crsfCallback).
// RX: non-blocking byte-by-byte parser for incoming telemetry frames.
//     ELRS sends LINK_STATISTICS (0x14) and BATTERY_SENSOR (0x08) frames
//     back on the same wire whenever telemetry is enabled in the module.
//
// Half-duplex wiring: 1 kΩ resistor between Serial1 TX and Serial1 RX.
// No direction-control pin needed — the resistor handles bus contention.
//
// References:
//   https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md
//   https://www.expresslrs.org/info/telemetry/

#include "crsf.h"

// ── Public state ──────────────────────────────────────────────────────────────
uint8_t       crsfPacket[CRSF_PACKET_SIZE];
int           rcChannels[CRSF_MAX_CHANNEL];
uint32_t      crsfTime        = 0;
CrsfLinkStats crsfLink        = {};
CrsfBattery   crsfBattery     = {};
bool          crsfLinkUpdated    = false;
bool          crsfBatteryUpdated = false;
uint32_t      crsfRxBytes        = 0;   // total raw bytes seen on Serial1
uint32_t      crsfFramesOk       = 0;   // frames that passed CRC
uint32_t      crsfFramesBadCrc   = 0;   // frames with wrong CRC

// ── CRC-8/DVB-S2 lookup table ─────────────────────────────────────────────────
static const uint8_t crc8tab[256] = {
    0x00,0xD5,0x7F,0xAA,0xFE,0x2B,0x81,0x54,0x29,0xFC,0x56,0x83,0xD7,0x02,0xA8,0x7D,
    0x52,0x87,0x2D,0xF8,0xAC,0x79,0xD3,0x06,0x7B,0xAE,0x04,0xD1,0x85,0x50,0xFA,0x2F,
    0xA4,0x71,0xDB,0x0E,0x5A,0x8F,0x25,0xF0,0x8D,0x58,0xF2,0x27,0x73,0xA6,0x0C,0xD9,
    0xF6,0x23,0x89,0x5C,0x08,0xDD,0x77,0xA2,0xDF,0x0A,0xA0,0x75,0x21,0xF4,0x5E,0x8B,
    0x9D,0x48,0xE2,0x37,0x63,0xB6,0x1C,0xC9,0xB4,0x61,0xCB,0x1E,0x4A,0x9F,0x35,0xE0,
    0xCF,0x1A,0xB0,0x65,0x31,0xE4,0x4E,0x9B,0xE6,0x33,0x99,0x4C,0x18,0xCD,0x67,0xB2,
    0x39,0xEC,0x46,0x93,0xC7,0x12,0xB8,0x6D,0x10,0xC5,0x6F,0xBA,0xEE,0x3B,0x91,0x44,
    0x6B,0xBE,0x14,0xC1,0x95,0x40,0xEA,0x3F,0x42,0x97,0x3D,0xE8,0xBC,0x69,0xC3,0x16,
    0xEF,0x3A,0x90,0x45,0x11,0xC4,0x6E,0xBB,0xC6,0x13,0xB9,0x6C,0x38,0xED,0x47,0x92,
    0xBD,0x68,0xC2,0x17,0x43,0x96,0x3C,0xE9,0x94,0x41,0xEB,0x3E,0x6A,0xBF,0x15,0xC0,
    0x4B,0x9E,0x34,0xE1,0xB5,0x60,0xCA,0x1F,0x62,0xB7,0x1D,0xC8,0x9C,0x49,0xE3,0x36,
    0x19,0xCC,0x66,0xB3,0xE7,0x32,0x98,0x4D,0x30,0xE5,0x4F,0x9A,0xCE,0x1B,0xB1,0x64,
    0x72,0xA7,0x0D,0xD8,0x8C,0x59,0xF3,0x26,0x5B,0x8E,0x24,0xF1,0xA5,0x70,0xDA,0x0F,
    0x20,0xF5,0x5F,0x8A,0xDE,0x0B,0xA1,0x74,0x09,0xDC,0x76,0xA3,0xF7,0x22,0x88,0x5D,
    0xD6,0x03,0xA9,0x7C,0x28,0xFD,0x57,0x82,0xFF,0x2A,0x80,0x55,0x01,0xD4,0x7E,0xAB,
    0x84,0x51,0xFB,0x2E,0x7A,0xAF,0x05,0xD0,0xAD,0x78,0xD2,0x07,0x53,0x86,0x2C,0xF9
};

static uint8_t crsf_crc8(const uint8_t *ptr, uint8_t len) {
    uint8_t crc = 0;
    while (len--) crc = crc8tab[crc ^ *ptr++];
    return crc;
}

// ── RX state (declared here so crsfSendPacket can reset the parser) ─────────
static uint8_t rxBuf[CRSF_FRAME_SIZE_MAX];
static uint8_t rxPos    = 0;
static uint8_t rxExpect = 0;

// ── TX ────────────────────────────────────────────────────────────────────────
void crsfInit() {
    // Serial1 defaults to GP0=TX, GP1=RX on RP2040 — matches the PCB wiring.
    // A 1kΩ resistor between GP0 and GP1 feeds the module's telemetry back
    // into RX. The loopback of our own TX bytes is harmless: 0xEE is not a
    // recognised destination address so those bytes are discarded immediately.
    Serial1.begin(CRSF_BAUDRATE);
    for (uint8_t i = 0; i < CRSF_MAX_CHANNEL; i++)
        rcChannels[i] = CRSF_CHANNEL_MID;
    rcChannels[THROTTLE] = CRSF_CHANNEL_MIN;
    crsfTime = micros();
}

void crsfPreparePacket(uint8_t packet[], int channels[]) {
    // Frame: [addr][len][type][22 bytes packed channels][crc]
    packet[0]  = CRSF_ADDR_TX_MODULE;
    packet[1]  = 24; // len = type(1) + payload(22) + crc(1)
    packet[2]  = CRSF_FRAMETYPE_RC_CHANNELS;

    // Pack 16 × 11-bit values into 22 bytes, LSB first
    packet[3]  = (uint8_t)( channels[0]        & 0x7FF);
    packet[4]  = (uint8_t)((channels[0] >> 8)  | (channels[1] << 3));
    packet[5]  = (uint8_t)((channels[1] >> 5)  | (channels[2] << 6));
    packet[6]  = (uint8_t)( channels[2] >> 2);
    packet[7]  = (uint8_t)((channels[2] >> 10) | (channels[3] << 1));
    packet[8]  = (uint8_t)((channels[3] >> 7)  | (channels[4] << 4));
    packet[9]  = (uint8_t)((channels[4] >> 4)  | (channels[5] << 7));
    packet[10] = (uint8_t)( channels[5] >> 1);
    packet[11] = (uint8_t)((channels[5] >> 9)  | (channels[6] << 2));
    packet[12] = (uint8_t)((channels[6] >> 6)  | (channels[7] << 5));
    packet[13] = (uint8_t)( channels[7] >> 3);
    packet[14] = (uint8_t)( channels[8]        & 0x7FF);
    packet[15] = (uint8_t)((channels[8] >> 8)  | (channels[9]  << 3));
    packet[16] = (uint8_t)((channels[9] >> 5)  | (channels[10] << 6));
    packet[17] = (uint8_t)( channels[10] >> 2);
    packet[18] = (uint8_t)((channels[10] >> 10)| (channels[11] << 1));
    packet[19] = (uint8_t)((channels[11] >> 7) | (channels[12] << 4));
    packet[20] = (uint8_t)((channels[12] >> 4) | (channels[13] << 7));
    packet[21] = (uint8_t)( channels[13] >> 1);
    packet[22] = (uint8_t)((channels[13] >> 9) | (channels[14] << 2));
    packet[23] = (uint8_t)((channels[14] >> 6) | (channels[15] << 5));
    packet[24] = (uint8_t)( channels[15] >> 3);
    packet[25] = crsf_crc8(&packet[2], CRSF_PACKET_SIZE - 3); // crc over type+payload
}

void crsfSendPacket() {
    crsfPreparePacket(crsfPacket, rcChannels);
    Serial1.write(crsfPacket, CRSF_PACKET_SIZE);
    // Our TX bytes echo back through the 1kΩ resistor into RX.
    // Flush them now before the module has time to respond, so the
    // parser never sees them. The module needs ~200µs to respond after
    // receiving our packet, so this flush is safe.
    Serial1.flush();          // wait until TX FIFO is drained
    while (Serial1.available()) Serial1.read(); // discard loopback bytes
    rxPos    = 0;             // reset parser — any partial frame was loopback garbage
    rxExpect = 0;
}

void crsfCallback() {
    if (micros() >= crsfTime) {
        crsfSendPacket();
        crsfTime = micros() + CRSF_TIME_BETWEEN_FRAMES_US;
    }
}

// ── RX parser ─────────────────────────────────────────────────────────────────
// CRSF frame layout from the module to us:
//   [dest_addr][frame_len][type][...payload...][crc]
//   frame_len = type(1) + payload(N) + crc(1)
//   total wire bytes = 2 + frame_len
//
// We wait for CRSF_ADDR_REMOTE (0xEA) as the destination.
// The state machine resyncs on any framing error.

static void parseFrame() {
    // rxBuf[0] = dest addr (already verified == CRSF_ADDR_REMOTE)
    // rxBuf[1] = frame_len (type + payload + crc)
    // rxBuf[2] = type
    // rxBuf[3..N-1] = payload
    // rxBuf[N] = crc

    uint8_t frameLen = rxBuf[1];
    uint8_t type     = rxBuf[2];
    uint8_t crcReceived = rxBuf[1 + frameLen]; // last byte

    // CRC covers type + payload (not addr, not len, not crc itself)
    uint8_t crcCalc = crsf_crc8(&rxBuf[2], frameLen - 1);
    if (crcCalc != crcReceived) { crsfFramesBadCrc++; return; }
    crsfFramesOk++;

    const uint8_t *p = &rxBuf[3]; // payload start

    if (type == CRSF_FRAMETYPE_LINK_STATS && frameLen >= 11) {
        // Dump the raw frame once so we can verify byte layout in the serial monitor.
        // frameLen = type(1) + payload(N) + crc(1), so payload is frameLen-2 bytes.
        // Remove this block once RSSI values look correct.
        // Raw frame dump — prints every 200 good frames so you can see live values.
        if (crsfFramesOk % 200 == 1) {
            Serial.print("LSTATS fl="); Serial.print(frameLen);
            Serial.print(" bytes:");
            for (uint8_t i = 0; i < frameLen - 1 && i < 12; i++) {
                Serial.print(' '); Serial.print(p[i], DEC);
                Serial.print('('); Serial.print(p[i], HEX); Serial.print(')');
            }
            Serial.println();
        }
        crsfLink.rssi_ant1      = (int8_t)p[0];  // stored as signed dBm
        crsfLink.rssi_ant2      = (int8_t)p[1];
        crsfLink.link_quality   = p[2];
        crsfLink.snr            = (int8_t)p[3];
        crsfLink.active_antenna = p[4];
        crsfLink.rf_mode        = p[5];
        crsfLink.tx_power       = p[6];
        crsfLink.downlink_rssi  = (int8_t)p[7];
        crsfLink.downlink_lq    = p[8];
        crsfLink.downlink_snr   = (int8_t)p[9];
        crsfLinkUpdated = true;
    }
    else if (type == CRSF_FRAMETYPE_BATTERY && frameLen >= 9) {
        // Payload: 8 bytes
        //  [0..1] voltage in 100mV steps (big-endian uint16)
        //  [2..3] current in 100mA steps (big-endian uint16)
        //  [4..6] capacity used in mAh (big-endian uint24)
        //  [7]    remaining battery %
        crsfBattery.voltage       = ((uint16_t)p[0] << 8 | p[1]) * 0.1f;
        crsfBattery.current       = ((uint16_t)p[2] << 8 | p[3]) * 0.1f;
        crsfBattery.capacity_used = ((uint32_t)p[4] << 16) | ((uint32_t)p[5] << 8) | p[6];
        crsfBattery.percent       = p[7];
        crsfBatteryUpdated = true;
    }
    // Other frame types are silently ignored.
}

bool crsfProcessRx() {
    bool gotFrame = false;

    while (Serial1.available()) {
        uint8_t b = Serial1.read();
        crsfRxBytes++;
#ifdef CRSF_DEBUG_RAW
        // Print every incoming byte as hex — use this to confirm the resistor
        // is working and bytes are actually arriving from the ELRS module.
        // Enable by adding -DCRSF_DEBUG_RAW to build_flags in platformio.ini.
        Serial.print(b, HEX); Serial.print(' ');
#endif

        if (rxPos == 0) {
            // ELRS sends telemetry frames to 0xC8 (flight controller address),
            // not 0xEA. Accept all three known destination addresses.
            if (b != CRSF_ADDR_FLIGHT_CTRL &&
                b != CRSF_ADDR_REMOTE      &&
                b != CRSF_ADDR_BROADCAST) continue;
            rxBuf[0] = b;
            rxPos = 1;
            rxExpect = 0;
        }
        else if (rxPos == 1) {
            // Frame length byte: type(1) + payload + crc(1)
            // Sanity check: ELRS telemetry frames are at most ~14 bytes
            if (b < 2 || b > CRSF_FRAME_SIZE_MAX - 2) {
                rxPos = 0; // resync
                continue;
            }
            rxBuf[1]  = b;
            rxExpect  = b + 2; // total wire bytes = addr + len + (type+payload+crc)
            rxPos     = 2;
        }
        else {
            rxBuf[rxPos++] = b;
            if (rxPos >= rxExpect) {
                parseFrame();
                gotFrame = true;
                rxPos    = 0;
                rxExpect = 0;
            }
        }
    }
    return gotFrame;
}