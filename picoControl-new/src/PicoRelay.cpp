#include <math.h>
#include "config.h"
#include "PicoRelay.h"

PicoRelay::PicoRelay()
#if defined(USE_9685)
    : pwm()
#elif defined(USE_9635)
    : pwm(0x70)
#endif
{
#if defined(LUMI)
    joystickActive = false;
#endif
}

void PicoRelay::begin() {
#if defined(USE_9685)
    pwm.begin();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(16000);
    for (int i = 0; i < 16; i++) {
        writeRelay(i, false);  // false = relay OFF = output HIGH
    }

#elif defined(USE_9635)
    // PCA9635 has OPEN-DRAIN outputs:
    // LEDOFF (00) = floating (HIGH via external pull-up) = relay OFF
    // LEDON  (01) = driven LOW (sinks current)           = relay ON
    // Low-active relays: deactivated = floating/HIGH = PCA963X_LEDOFF
    // writeRelay(i, true)  → LEDON  → LOW  → relay ON  ✓
    // writeRelay(i, false) → LEDOFF → HIGH → relay OFF ✓
    // Must call setLedDriverMode() for all channels after begin() or
    // LEDOUT registers are uninitialised.
    Serial.println("9635 init");
    int rv = pwm.begin();
    Serial.print("9635 begin rv="); Serial.println(rv);
    if (rv == 0) {
        for (int i = 0; i < 16; i++) {
            pwm.setLedDriverMode(i, PCA963X_LEDOFF);  // floating/HIGH — relay deactivated
        }
        Serial.println("9635 ready, all outputs floating/HIGH (relays off)");
        // Flash each relay in sequence to verify wiring
        Serial.println("9635 relay test start");
        for (int i = 0; i < 16; i++) {
            pwm.setLedDriverMode(i, PCA963X_LEDON);  // LOW (sink) — relay ON
            delay(150);
            pwm.setLedDriverMode(i, PCA963X_LEDOFF); // floating   — relay OFF
            delay(100);
            Serial.print("relay "); Serial.print(i); Serial.println(" ok");
        }
        Serial.println("9635 relay test done");
    } else {
        Serial.println("9635 begin FAILED - check I2C address/wiring");
    }
#endif

#if defined(EXTRA_RELAY)
    for (int i = 0; i < 8; i++) {
        pinMode(relaypin[i], OUTPUT);
        digitalWrite(relaypin[i], HIGH);  // HIGH = relay deactivated (low-active)
    }
#endif
}

void PicoRelay::writeRelay(int relaynr, bool state) {
    if (relaynr < 0 || relaynr >= 24) return;

    if (relaynr < 16) {
#if defined(USE_9685)
        // state=true  → relay ON  → output LOW  → PWM=0
        // state=false → relay OFF → output HIGH → PWM=4095
        pwm.setPWM(relaynr, 0, state ? 0 : 4095);
#elif defined(USE_9635)
        // state=true  → relay ON  → output LOW  → LEDON  (open-drain sink)
        // state=false → relay OFF → floating    → LEDOFF (open-drain off)
        pwm.setLedDriverMode(relaynr, state ? PCA963X_LEDON : PCA963X_LEDOFF);
#endif
    }
#if defined(EXTRA_RELAY)
    else if (relaynr < 24) {
        // Low-active: state=true → LOW, state=false → HIGH
        digitalWrite(relaypin[relaynr - 16], state ? LOW : HIGH);
    }
#endif
}

#if defined(LUMI)
const uint8_t PicoRelay::driveRelays[12] = {
    0b00001000, 0b00011000, 0b00010000, 0b00110000,
    0b00100000, 0b00100001, 0b00000001, 0b00000011,
    0b00000010, 0b00000110, 0b00000100, 0b00001100
};

void PicoRelay::joystickToRelays(int x, int y) {
    const int center = 127;
    const int enterThreshold = 60;
    const int exitThreshold = 40;

    int dx = x - center;
    int dy = y - center;
    int distance = sqrt(dx * dx + dy * dy);

    if (!joystickActive && distance > enterThreshold) {
        joystickActive = true;
    } else if (joystickActive && distance < exitThreshold) {
        joystickActive = false;
    }

    if (joystickActive) {
        int relayNumber = constrain((180 + 360.0 * (atan2(dx, dy) / (2 * PI))) / 30, 0, 11);
        for (int i = 0; i < 6; i++) {
            writeRelay(i, driveRelays[relayNumber] & (1 << i));
        }
    } else {
        for (int i = 0; i < 6; i++) {
            writeRelay(i, LOW);
        }
    }
}
#endif

void PicoRelay::relayTest() {
#if defined(USE_9685)
    Serial.println("relayTest V1 (9685): I2C ping at 0x40...");
    Wire.setClock(100000);
    Wire.beginTransmission(0x40);
    int err = Wire.endTransmission();
    if (err != 0) {
        Serial.print("NACK at 0x40 (err="); Serial.print(err);
        Serial.println(") - chip not responding, skipping relay flash");
        return;
    }
    Serial.println("ACK - flashing relays 0-15");
    for (int i = 0; i < 16; i++) {
        Serial.print("  relay "); Serial.print(i); Serial.print("...");
        pwm.setPWM(i, 0, 0);     // LOW = ON
        delay(200);
        pwm.setPWM(i, 0, 4095);  // HIGH = OFF
        delay(100);
        Serial.println(" done");
    }
    Serial.println("relayTest V1 done");

#elif defined(USE_9635)
    Serial.println("relayTest V2 (9635): I2C ping...");
    Wire.setClock(100000);
    Wire.beginTransmission(0x70);
    int err = Wire.endTransmission();
    if (err != 0) {
        Serial.print("NACK at 0x70 (err="); Serial.print(err);
        Serial.println(") - chip not responding, skipping relay flash");
        return;
    }
    Serial.println("ACK - flashing relays 0-15");
    for (int i = 0; i < 16; i++) {
        Serial.print("  relay "); Serial.print(i); Serial.print("...");
        pwm.setLedDriverMode(i, PCA963X_LEDON);   // LOW (sink) = relay ON
        delay(200);
        pwm.setLedDriverMode(i, PCA963X_LEDOFF);  // floating   = relay OFF
        delay(100);
        Serial.println(" done");
    }
    Serial.println("relayTest V2 done");

#else
    Serial.println("relayTest: no relay chip defined for this board");
#endif
}

void PicoRelay::relayStatus() {
#if defined(USE_9685) || defined(USE_9635)
#if defined(USE_9685)
    Serial.println("Board V1 (PCA9685) - I2C scan:");
#else
    Serial.println("Board V2 (PCA9635) - I2C scan:");
#endif
    uint32_t clocks[2] = {100000UL, 10000UL};
    for (int c = 0; c < 2; c++) {
        Wire.setClock(clocks[c]);
        Serial.print("  at "); Serial.print(clocks[c]/1000); Serial.println("kHz:");
        bool found = false;
        for (uint8_t addr = 1; addr < 127; addr++) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                Serial.print("    found 0x"); Serial.println(addr, HEX);
                found = true;
            }
        }
        if (!found) Serial.println("    nothing found");
    }
    Wire.setClock(400000);
    Serial.println("  scan done");
#else
    Serial.println("relayStatus: no relay chip defined for this board");
#endif
}