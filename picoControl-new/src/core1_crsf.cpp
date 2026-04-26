#include "config.h"
#ifdef USE_CRSF

#include "core1_crsf.h"
#include <pico/mutex.h>

// CRSF channel range (11-bit values from the ELRS receiver)
#define CRSF_CHANNEL_MIN  172
#define CRSF_CHANNEL_MAX  1811

// ---------------------------------------------------------------------------
// CRSF frame constants
// ELRS RC frame: [0xC8][len=24][type=0x16][22 bytes payload][crc]
// Total on wire: 26 bytes
// ---------------------------------------------------------------------------
#define CRSF_ADDR           0xC8
#define CRSF_TYPE_RC        0x16
#define CRSF_EXPECTED_LEN   24      // value in the length field for an RC frame

// ---------------------------------------------------------------------------
// Link state machine
// ---------------------------------------------------------------------------
enum LinkState { LINK_WAITING, LINK_ACTIVE, LINK_LOST };

// ---------------------------------------------------------------------------
// Shared state — written by Core 1, read by Core 0 under mutex
// ---------------------------------------------------------------------------
static mutex_t    _mtx;
static int        _channels[16]  = {0};
static LinkState  _linkState     = LINK_WAITING;
static uint32_t   _lastFrameMs   = 0;
static uint32_t   _frameCount    = 0;
static uint32_t   _crcErrors     = 0;
static uint32_t   _restartCount  = 0;
static uint32_t   _bytesReceived = 0;

// ---------------------------------------------------------------------------
// Core 0 interface
// ---------------------------------------------------------------------------
bool crsfReady()                          { return _linkState == LINK_ACTIVE; }
bool crsfLost()                           { return _linkState == LINK_LOST; }
uint32_t crsfSilenceMs()                  { return (_lastFrameMs == 0) ? 0 : millis() - _lastFrameMs; }

void crsfGetChannels(int* dst, int count) {
    mutex_enter_blocking(&_mtx);
    for (int i = 0; i < count && i < 16; i++) dst[i] = _channels[i];
    mutex_exit(&_mtx);
}

CrsfStatus crsfGetStatus() {
    CrsfStatus s;
    mutex_enter_blocking(&_mtx);
    s.linkState  = (int)_linkState;
    s.silenceMs  = (_lastFrameMs == 0) ? 0 : millis() - _lastFrameMs;
    s.frameCount = _frameCount;
    s.crcErrors  = _crcErrors;
    s.restarts   = _restartCount;
    mutex_exit(&_mtx);
    return s;
}

// ---------------------------------------------------------------------------
// CRC-8 (DVB-S2) — same table as CRSF.cpp
// ---------------------------------------------------------------------------
static const uint8_t _crcTable[256] = {
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

static uint8_t _crc8(const uint8_t* buf, int len) {
    uint8_t crc = 0;
    for (int i = 0; i < len; i++) crc = _crcTable[crc ^ buf[i]];
    return crc;
}

// ---------------------------------------------------------------------------
// Private parser state
// ---------------------------------------------------------------------------
static uint8_t  _buf[64];
static int      _bufIdx         = 0;
static int      _frameLen       = 0;
static uint32_t _restartAttempt = 0;
static uint32_t _lostSinceMs    = 0;
static uint32_t _diagLastPrintMs= 0;

static void _parseBytes() {
    while (Serial2.available()) {
        uint8_t b = Serial2.read();
        _bytesReceived++;

        if (_bufIdx == 0) {
            // Waiting for address byte
            if (b == CRSF_ADDR) {
                _buf[_bufIdx++] = b;
            }
            // any other byte: stay at 0, keep hunting
        }
        else if (_bufIdx == 1) {
            // Length field — must be reasonable
            _frameLen = b;
            if (_frameLen < 2 || _frameLen > 62) {
                // Invalid length — discard and resync
                _bufIdx = 0;
            } else {
                _buf[_bufIdx++] = b;
            }
        }
        else {
            // Accumulate payload + CRC
            _buf[_bufIdx++] = b;

            // Full frame = addr(1) + len(1) + payload(_frameLen-1) + crc(1)
            // = _frameLen + 2 bytes total
            if (_bufIdx == _frameLen + 2) {
                // Verify CRC over [type ... payload] = bytes [2 .. _frameLen]
                uint8_t expected = _crc8(&_buf[2], _frameLen - 1);
                uint8_t received = _buf[_frameLen + 1];

                if (expected == received && _buf[1] == CRSF_EXPECTED_LEN
                                        && _buf[2] == CRSF_TYPE_RC) {
                    // Valid RC frame — unpack 16 channels (11-bit each)
                    const uint8_t* d = &_buf[3];
                    int16_t ch[16];
                    ch[0]  = ((d[0]      | d[1]  << 8) & 0x7FF);
                    ch[1]  = ((d[1]  >> 3| d[2]  << 5) & 0x7FF);
                    ch[2]  = ((d[2]  >> 6| d[3]  << 2| d[4]  << 10) & 0x7FF);
                    ch[3]  = ((d[4]  >> 1| d[5]  << 7) & 0x7FF);
                    ch[4]  = ((d[5]  >> 4| d[6]  << 4) & 0x7FF);
                    ch[5]  = ((d[6]  >> 7| d[7]  << 1| d[8]  <<  9) & 0x7FF);
                    ch[6]  = ((d[8]  >> 2| d[9]  << 6) & 0x7FF);
                    ch[7]  = ((d[9]  >> 5| d[10] << 3) & 0x7FF);
                    ch[8]  = ((d[11]     | d[12] << 8) & 0x7FF);
                    ch[9]  = ((d[12] >> 3| d[13] << 5) & 0x7FF);
                    ch[10] = ((d[13] >> 6| d[14] << 2| d[15] << 10) & 0x7FF);
                    ch[11] = ((d[15] >> 1| d[16] << 7) & 0x7FF);
                    ch[12] = ((d[16] >> 4| d[17] << 4) & 0x7FF);
                    ch[13] = ((d[17] >> 7| d[18] << 1| d[19] <<  9) & 0x7FF);
                    ch[14] = ((d[19] >> 2| d[20] << 6) & 0x7FF);
                    ch[15] = ((d[20] >> 5| d[21] << 3) & 0x7FF);

                    // Map to 0-255 and publish atomically
                    int mapped[16];
                    for (int n = 0; n < 16; n++) {
                        mapped[n] = constrain(
                            map(ch[n],
                                CRSF_CHANNEL_MIN - CRSF_CHANNEL_OFFSET,
                                CRSF_CHANNEL_MAX - CRSF_CHANNEL_OFFSET,
                                0, 255), 0, 255);
                    }
                    // ch[8] uses a slightly different upper bound
                    mapped[8] = constrain(
                        map(ch[8],
                            CRSF_CHANNEL_MIN - CRSF_CHANNEL_OFFSET,
                            CRSF_CHANNEL_MAX, 0, 255), 0, 255);

                    mutex_enter_blocking(&_mtx);
                    for (int n = 0; n < 16; n++) _channels[n] = mapped[n];
                    _frameCount++;
                    _lastFrameMs = millis();
                    bool wasLost = (_linkState != LINK_ACTIVE);
                    _linkState   = LINK_ACTIVE;
                    mutex_exit(&_mtx);

                    // Concise state-change messages only
                    if (_frameCount == 1) {
                        Serial.println("CRSF: ACTIVE (first frame)");
                    } else if (wasLost) {
                        Serial.print("CRSF: ACTIVE (recovered after ");
                        Serial.print(_restartAttempt);
                        Serial.println(" restarts)");
                        _restartAttempt = 0;
                        _lostSinceMs    = 0;
                    }

                } else if (expected != received) {
                    mutex_enter_blocking(&_mtx);
                    _crcErrors++;
                    mutex_exit(&_mtx);
                }
                // Reset for next frame regardless
                _bufIdx = 0;
            }

            // Safety: if buffer overflows without completing, resync
            if (_bufIdx >= (int)sizeof(_buf)) {
                _bufIdx = 0;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Serial init
// ---------------------------------------------------------------------------
static void _startSerial() {
    Serial2.setTX(8);
    Serial2.setRX(9);
    Serial2.begin(420000);
    while (Serial2.available()) Serial2.read();  // flush stale bytes
    _bufIdx = 0;
}

static void _doRestart(const char* reason) {
    Serial2.end();
    delay(200);           // give ELRS receiver time to detect UART gone and reset
    _startSerial();
    delay(100);           // wait for receiver to detect new UART and start streaming
    _restartAttempt++;
    mutex_enter_blocking(&_mtx);
    _restartCount++;
    mutex_exit(&_mtx);
    Serial.print("CRSF RESTART #");
    Serial.print(_restartAttempt);
    Serial.print(" reason=");
    Serial.println(reason);
}

static void _printDiag(uint32_t intervalMs = 2000) {
    // Only used during WAITING state for initial feedback — elsewhere we print on transitions
    uint32_t now = millis();
    if (now - _diagLastPrintMs < intervalMs) return;
    _diagLastPrintMs = now;
    Serial.print("CRSF WAIT bytes="); Serial.print(_bytesReceived);
    Serial.print(" bufIdx="); Serial.println(_bufIdx);
}

// ---------------------------------------------------------------------------
// Core 1 setup / loop
// ---------------------------------------------------------------------------
void crsfCore1Setup() {
    mutex_init(&_mtx);
    _startSerial();
    Serial.println("CRSF Core1: setup done");
}

void crsfCore1Loop() {
    _parseBytes();

    uint32_t now = millis();

    switch (_linkState) {
        case LINK_WAITING:
            _printDiag();
            // If no bytes at all after 5s, restart serial (wrong pins/wiring)
            if (now > 5000 && _bytesReceived == 0) {
                _doRestart("no bytes in 5s");
            }
            break;

        case LINK_ACTIVE:
            if (crsfSilenceMs() > 500) {
                Serial.println("CRSF: LOST");
                _lostSinceMs = now;
                mutex_enter_blocking(&_mtx);
                _linkState = LINK_LOST;
                mutex_exit(&_mtx);
            }
            break;

        case LINK_LOST: {
            uint32_t interval = (_restartAttempt == 0) ? 600
                              : min(3000UL, 500UL * _restartAttempt);
            if (now - _lostSinceMs > interval) {
                _lostSinceMs = now;
                _doRestart("restart");
            }
            break;
        }
    }
}

#endif // USE_CRSF