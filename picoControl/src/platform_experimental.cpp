#ifdef EXPERIMENTAL

#include "config.h"
#include "platform.h"
#include "Motor.h"
#include "ddsm_ctrl.h"
#include "M5Unit8Servos.h"
#include <SoftwareSerial.h>

const int saveValues[] = { 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//                          X1   Y1  X2  Y2
//                               ^mouth=0->1150us(closed)

extern M5Unit8Servos servos;

DDSM_CTRL dc;
SoftwareSerial DDSMport(15, 14);   // RX=GP15, TX=GP14

#define DDSM_ID       4
#define DDSM_MAX_RPM  2100
#define DEADBAND      15
#define DDSM_PWR_PIN  11    // MOSFET gate — HIGH=power on, LOW=power cut

Motor motorRight(18, 19, 20, -1);
Motor motorLeft (21, 22, 26, -1);

void configureMotors() {
    motorLeft.init();
    motorRight.init();
}

void platformSetup() {
    configureMotors();

    pinMode(DDSM_PWR_PIN, OUTPUT);
    digitalWrite(DDSM_PWR_PIN, HIGH);  // N-channel: HIGH = motor ON
    DDSMport.begin(115200);
    dc.pSerial = &DDSMport;
    dc.set_ddsm_type(210);
    dc.clear_ddsm_buffer();
    delay(200);
    for (int i = 0; i < 3; i++) {
        dc.ddsm_change_mode(DDSM_ID, 2);
        delay(20);
        dc.ddsm_ctrl(DDSM_ID, 0, 0);
        delay(20);
    }
    dc.clear_ddsm_buffer();

    servos.writeServoPulse(0, 1150, true);  // mouth closed
}

void platformLoop() {
    extern int channels[];

    // ── DDSM210 — left stick Y (channels[3]) ─────────────────────────────────
    int yRaw    = channels[CRSF_CH_AXIS_Y2] - 127;
    bool inDeadband = (yRaw > -DEADBAND && yRaw < DEADBAND);
    static bool wasInDeadband = true;

    static bool  powerCut         = false;
    static unsigned long restoreTime = 0;

    // SA (channels[4]): 3=free-spin in deadband (default), 252=lock at 0 rpm
    bool lockMode = (channels[4] > 128);

    if (inDeadband) {
        if (lockMode) {
            // Lock mode — hold at 0 rpm in speed loop (original behaviour)
            if (powerCut) {
                // Restore power if it was cut
                digitalWrite(DDSM_PWR_PIN, HIGH);
                DDSMport.begin(115200);
                powerCut    = false;
                restoreTime = 0;
                wasInDeadband = true;
            }
            if (wasInDeadband) {
                dc.ddsm_change_mode(DDSM_ID, 2);
                wasInDeadband = false;
            }
            dc.ddsm_ctrl(DDSM_ID, 0, 0);
        } else {
            // Free-spin mode — cut motor power
            if (!powerCut) {
                digitalWrite(DDSM_PWR_PIN, LOW);
                DDSMport.end();
                pinMode(14, INPUT);
                powerCut      = true;
                restoreTime   = 0;
            }
        }
        wasInDeadband = true;

    } else {
        if (powerCut) {
            // Restore power and start non-blocking boot wait
            digitalWrite(DDSM_PWR_PIN, HIGH);  // N-channel: HIGH = motor ON
            DDSMport.begin(115200);               // re-init SoftwareSerial
            restoreTime = millis();
            powerCut    = false;
        }

        if (restoreTime > 0) {
            // Still waiting for DDSM to boot
            if (millis() - restoreTime < 80) {   // tune down until unreliable
                wasInDeadband = true;  // keep wasInDeadband true so mode re-asserts
                return;                // skip rest of loop — don't drive yet
            }
            // Boot complete — re-init DDSM
            dc.ddsm_change_mode(DDSM_ID, 2);
            dc.ddsm_ctrl(DDSM_ID, 0, 0);
            restoreTime   = 0;
            wasInDeadband = true;  // force mode re-assert on next tick
        }

        if (wasInDeadband) {
            dc.ddsm_change_mode(DDSM_ID, 2);
            wasInDeadband = false;
        }
        int rpm = map(yRaw, -127, 127, -DDSM_MAX_RPM, DDSM_MAX_RPM);
        dc.ddsm_ctrl(DDSM_ID, rpm, 0);
    }

    // ── Debug: print parsed feedback from last ddsm_ctrl() call ─────────────
    // ddsm_ctrl() calls ddsm210_fb() internally which populates dc fields.
    // Also call ddsm_get_info() every 500ms to get position (0x74 packet).
    static unsigned long lastDbg = 0;
    if (millis() - lastDbg > 500) {
        // Get info packet for position
        dc.ddsm_get_info(DDSM_ID);

        Serial.print(lockMode ? F("LOCK ") : (powerCut ? F("OFF  ") : (wasInDeadband ? F("BOOT ") : F("SPD  "))));
        Serial.print(F("spd="));  Serial.print(dc.speed_data);
        Serial.print(F(" cur="));  Serial.print(dc.current);
        Serial.print(F(" pos="));  Serial.print(dc.ddsm_pos);
        Serial.print(F(" tmp="));  Serial.print(dc.temperature);
        Serial.print(F(" flt="));  Serial.println(dc.fault_code);
        lastDbg = millis();
    }

    // ── Mouth servo — left stick X (channels[2]) ─────────────────────────────
    // 1150us = closed, 800us = open
    int mouthPulse = map(channels[CRSF_CH_AXIS_X2], 0, 255, 1150, 800);
    servos.writeServoPulse(0, mouthPulse, true);
}

void platformScreen() {}

void platformOnIdle() {
    dc.ddsm_ctrl(DDSM_ID, 0, 0);
}

void platformSetup1()    {}
void platformLoopCore1() {}

#endif // EXPERIMENTAL