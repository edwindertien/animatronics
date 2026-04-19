#include "config.h"
#ifdef USE_CRSF

#include "core1_crsf.h"
#include "CRSF.h"
#include <pico/mutex.h>

// ---------------------------------------------------------------------------
// Shared state — written by Core 1, read by Core 0 under spinlock
// ---------------------------------------------------------------------------
static mutex_t    _mtx;
static int        _channels[16] = {0};   // mapped 0-255
static bool       _ready        = false;
static uint32_t   _lastFrameMs  = 0;

// ---------------------------------------------------------------------------
// Private Core 1 state
// ---------------------------------------------------------------------------
static CRSF _crsf;

// ---------------------------------------------------------------------------
// Core 0 interface
// ---------------------------------------------------------------------------
bool crsfReady() {
    return _ready;
}

uint32_t crsfSilenceMs() {
    return millis() - _lastFrameMs;
}

void crsfGetChannels(int* dst, int count) {
    mutex_enter_blocking(&_mtx);
    for (int i = 0; i < count && i < 16; i++) dst[i] = _channels[i];
    mutex_exit(&_mtx);
}

// ---------------------------------------------------------------------------
// Core 1 functions
// ---------------------------------------------------------------------------
void crsfCore1Setup() {
    mutex_init(&_mtx);
    _crsf.begin();
    Serial.println("CRSF Core1: started");
}

void crsfCore1Loop() {
    _crsf.GetCrsfPacket();

    if (_crsf.failsafe_status == CRSF_SIGNAL_OK) {
        // Valid frame received — map and publish channels atomically
        int mapped[16];
        for (int n = 0; n < 16; n++) {
            mapped[n] = constrain(
                map(_crsf.channels[n],
                    CRSF_CHANNEL_MIN - CRSF_CHANNEL_OFFSET,
                    CRSF_CHANNEL_MAX - CRSF_CHANNEL_OFFSET,
                    0, 255),
                0, 255);
        }
        // channels[8] uses a slightly different upper bound (original behaviour)
        mapped[8] = constrain(
            map(_crsf.channels[8],
                CRSF_CHANNEL_MIN - CRSF_CHANNEL_OFFSET,
                CRSF_CHANNEL_MAX,
                0, 255),
            0, 255);

        mutex_enter_blocking(&_mtx);
        for (int n = 0; n < 16; n++) _channels[n] = mapped[n];
        mutex_exit(&_mtx);

        _lastFrameMs = millis();
        _ready       = true;
    }

    // --- Robustness: restart serial on sustained loss ---
    uint32_t silence = crsfSilenceMs();

    if (_ready && silence > 500 && silence < 600) {
        // First restart attempt after 500ms of silence
        Serial2.end();
        delay(10);
        _crsf.begin();
        Serial.println("CRSF Core1: restart after 500ms loss");
    }
    else if (_ready && silence > 2000 && (silence % 2000) < 10) {
        // Retry every 2s if still lost
        Serial2.end();
        delay(50);
        _crsf.begin();
        Serial.println("CRSF Core1: retry");
    }
}

#endif // USE_CRSF
