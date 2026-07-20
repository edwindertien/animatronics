#ifdef EXPERIMENTAL

#include "config.h"
#include "platform.h"
#include "Motor.h"
#include "M5Unit8Servos.h"
#include "core1_crsf.h"
#include "ak60.h"

const int saveValues[] = { 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//                          X1   Y1  X2  Y2
//                               ^mouth=0->1150us(closed)

extern M5Unit8Servos servos;

// DDSM removed for this build — only platform_stofzuiger.cpp drives it.
// Serial2 (UART1) is now exclusively CRSF's — see core1_crsf.cpp.

// AK60 CAN motor driver (protocol/hardware layer) lives in ak60.h/ak60.cpp,
// shared with platform_washmachine.cpp. Wiring: M5Stack CAN unit on GP2(TX)/
// GP3(RX), motor CAN ID 104 — see ak60.h for details/overrides.

#define AK60_POS_RANGE_DEG   30.0f   // full stick sweep = ±30°
#define AK60_ENGAGE_CH       CRSF_CH_ARM  // SA→ARM: disarmed=free, armed=position
#define AK60_SPD_LIMIT       5000    // electrical speed cap, raw int16 — CubeMars tool default
#define AK60_ACCEL_LIMIT     30000   // electrical accel cap, raw int16 — CubeMars tool default
#define AK60_USE_POS_SPD     0       // 1 = SET_POS_SPD (tunable spd/accel above), 0 = plain SET_POS (motor default profile)

// Soft re-engage: on disarmed→armed, approach the stick target gently
// (low spd/accel) instead of jumping straight to full power. Once close
// enough (or after a timeout, as a safety fallback), switch to normal
// full-power tracking above.
#define AK60_CATCHUP_SPD        500    // electrical speed cap while catching up — tune by feel
#define AK60_CATCHUP_ACCEL      2000   // electrical accel cap while catching up — tune by feel
#define AK60_CATCHUP_DONE_DEG   2.0f   // catch-up ends once within this many degrees of target
#define AK60_CATCHUP_TIMEOUT_MS 3000UL // safety fallback if the threshold is never reached

static bool ak60_wasEngaged  = false;
static bool ak60_catchingUp  = false;
static unsigned long ak60_catchupT0 = 0;

Motor motorRight(18, 19, 20, -1);
Motor motorLeft (21, 22, 26, -1);

void configureMotors() {
    motorLeft.init();
    motorRight.init();
}

void platformSetup() {
    configureMotors();

    servos.writeServoPulse(0, 1150, true);  // mouth closed
    // CAN init deferred to first platformLoop() tick
}

void platformLoop() {
    extern int channels[];

    // ── Loop/CRSF/CAN rate instrumentation — printed once per second ─────────
    static uint32_t loopCount      = 0;
    static uint32_t lastRateMs     = 0;
    static uint32_t lastLoopCount  = 0;
    static uint32_t lastFrameCount = 0;
    static uint32_t lastTxTotal    = 0;
    loopCount++;
    if (millis() - lastRateMs >= 1000) {
        uint32_t dt = millis() - lastRateMs;
        CrsfStatus crsf = crsfGetStatus();
        uint32_t txTotal = ak60Ready() ? ak60TxTotal() : 0;
        if (ak60DebugStream()) {
            float loopHz = (loopCount - lastLoopCount)   * 1000.0f / dt;
            float crsfHz = (crsf.frameCount - lastFrameCount) * 1000.0f / dt;
            float canHz  = (txTotal - lastTxTotal)       * 1000.0f / dt;
            Serial.print(F("[RATE] loop="));   Serial.print(loopHz, 1);
            Serial.print(F("Hz crsf="));       Serial.print(crsfHz, 1);
            Serial.print(F("Hz can_tx="));     Serial.print(canHz, 1);
            Serial.println(F("Hz"));
        }
        lastRateMs     = millis();
        lastLoopCount  = loopCount;
        lastFrameCount = crsf.frameCount;
        lastTxTotal    = txTotal;
    }

    // ── Deferred CAN init — once, after all resources allocated ──────────────
    ak60Begin();

    // ── AK60 — left stick X (channels[2]) → position control ─────────────────
    if (ak60Ready()) {
        float target = map(channels[CRSF_CH_AXIS_X2], 0, 255,
                            (int)(-AK60_POS_RANGE_DEG * 10), (int)(AK60_POS_RANGE_DEG * 10)) / 10.0f;
        bool engaged = channels[AK60_ENGAGE_CH] > 128;

        if (!engaged) {
            // Backdrivable: zero commanded current, shaft free to be moved by hand.
            ak60SetCurrent(0.0f);
            ak60_wasEngaged = false;
        } else {
            if (!ak60_wasEngaged) {
                // Rising edge (just re-engaged) — start the soft catch-up phase.
                ak60_catchingUp = true;
                ak60_catchupT0  = millis();
            }
            if (ak60_catchingUp) {
                float gap = target - ak60Pos();
                if (gap < 0) gap = -gap;
                if (gap < AK60_CATCHUP_DONE_DEG ||
                    millis() - ak60_catchupT0 > AK60_CATCHUP_TIMEOUT_MS) {
                    ak60_catchingUp = false;
                }
            }
            if (ak60_catchingUp) {
                // Soft approach — low speed/accel until close to the stick target.
                ak60SetPosSpd(target, AK60_CATCHUP_SPD, AK60_CATCHUP_ACCEL);
            } else {
                // Speed/accel-limited move — the motor's own trajectory generator
                // eases into position, following the live stick target.
#if AK60_USE_POS_SPD
                ak60SetPosSpd(target, AK60_SPD_LIMIT, AK60_ACCEL_LIMIT);
#else
                ak60SetPos(target);
#endif
            }
            ak60_wasEngaged = true;
        }

        ak60DebugPrint();
    }

    // ── Mouth servo — left stick X placeholder (channels[2]) ─────────────────
    // Mouth and AK60 share channels[2] for now — separate when hardware is split
    int mouthPulse = map(channels[CRSF_CH_AXIS_X2], 0, 255, 1150, 800);
    servos.writeServoPulse(0, mouthPulse, true);
}

void platformScreen() {}

void platformOnIdle() {
    // Servo-mode position control holds the last commanded position on its
    // own — no disable/idle command sent for now. Flag if different idle
    // behavior (e.g. current-brake to let it free-spin) is wanted here.
}

void platformSetup1()    {}
void platformLoopCore1() {}

#endif // EXPERIMENTAL