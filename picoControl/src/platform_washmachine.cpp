#ifdef WASHMACHINE

#include "config.h"
#include "platform.h"
#include "Motor.h"
#include "M5Unit8Servos.h"
#include "ak60.h"

const int saveValues[] = { 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

extern M5Unit8Servos servos;
extern int channels[];

// ============================================================================
// Select drive mode in config.h:
//   #define OMNI_DRIVE   — 4x omniwheel via HB25 on servo channels 0-3
//   (default)            — original Motor.h DC drive + trommel
// ============================================================================

#ifndef OMNI_DRIVE

// ----------------------------------------------------------------------------
// ORIGINAL configuration — DC motors + trommel + door servo
// ----------------------------------------------------------------------------
Motor motorRight(19, 22, 26, -1);
Motor motorLeft (21, 18, 20, -1);
Motor trommel   (-1, 15, 14, -1);

void configureMotors() {
    motorLeft.init();
    motorRight.init();
    trommel.init();
}

void platformSetup() {
    configureMotors();
}

void platformLoop() {
    extern bool brakeState;
    extern unsigned long brakeTimer;

    trommel.setSpeed(map(channels[3], 0, 255, -255, 255), brakeState);

    if (channels[7] == 252) {
        servos.writeServoPulse(7, map(channels[2], 0, 255, 2000, 1300), true);
        servos.writeServoPulse(6, map(channels[6], 0, 255, 1000, 2000), true);
    } else {
        servos.detach(0);
        servos.detach(7);
    }
}

void platformScreen()    {}
void platformOnIdle()    {}

#else // OMNI_DRIVE

// ============================================================================
// OMNI DRIVE configuration — 4x omniwheel, Parallax HB25 motor drivers
// ============================================================================
//
// Hardware: M5Unit8Servos board (I2C 0x25), 8 servo channels
// Motor driver: Parallax HB25 (servo pulse input)
//   1000us = full reverse, 1500us = stop, 2000us = full forward
//
// Servo channel assignment:
//   ch 0 = Front-Left  motor  (HB25)
//   ch 1 = Front-Right motor  (HB25)
//   ch 2 = Back-Left   motor  (HB25)
//   ch 3 = Back-Right  motor  (HB25)
//   ch 6 = Door servo  (moved from ch 0 to free up ch 0-3 for drive)
//
// Joystick/channel mapping:
//   channels[CRSF_CH_AXIS_Y1] = forward/backward  (127=centre)
//   channels[CRSF_CH_AXIS_X1] = strafe left/right (127=centre)
//   channels[CRSF_CH_AXIS_X2] = rotate CW/CCW     (127=centre)
//   channels[CRSF_CH_AXIS_Y2] = door servo        (full range 0-255)
//
// Omniwheel layout (viewed from above, front = direction of travel):
//
//         FRONT
//     FL(0)   FR(1)
//     BL(2)   BR(3)
//         BACK
//
// Mixing matrix:
//   FL =  Y + X + R
//   FR =  Y - X - R
//   BL =  Y - X + R
//   BR =  Y + X - R
//
// If a motor turns the wrong way, negate its output in omniDrive() below.
// Do not rewire — fix in software first.
//
// Safety:
//   - platformSetup(): all drive channels set to 1500us before CRSF starts
//   - platformOnIdle(): all drive channels set to 1500us on RF link loss
//   - Door servo retains last position on link loss (physically safe)
//   - Dead-band of +/-DEADBAND counts around centre suppresses motor noise
// ============================================================================

// Stub Motor instances and configureMotors() to satisfy main.cpp linker references.
// main.cpp calls configureMotors() in setup() and references motorLeft/motorRight
// in loop() for speed scaling — neither is used in omni drive mode.
// Motor(-1,-1,-1,-1) is valid — all pins disabled, setSpeed() is a no-op.
#include "Motor.h"
Motor motorLeft (-1, -1, -1, -1);
Motor motorRight(-1, -1, -1, -1);
Motor trommel   (-1, 15, 14, -1);
void configureMotors() {
      trommel.init();

}   // no-op — servo board init is in platformSetup()

#define HB25_STOP     1500
#define HB25_MAX_FWD  2000
#define HB25_MAX_REV  1000
#define HB25_RANGE     500

#define CH_FL    0
#define CH_FR    1
#define CH_BL    2
#define CH_BR    3
#define CH_DOOR  6
#define CH_KNOB  7

#define DEADBAND 4

// AK60 CAN motor (mouth) — tandem with the door servo for now, same input
// channel driving both. Reuses the tuning values proven out on EXPERIMENTAL.
#define AK60_POS_RANGE_DEG 30.0f   // full stick sweep = ±30°
#define AK60_SPD_LIMIT     5000    // electrical speed cap, raw int16 — CubeMars tool default
#define AK60_ACCEL_LIMIT   30000   // electrical accel cap, raw int16 — CubeMars tool default
#define AK60_USE_POS_SPD   0       // 1 = SET_POS_SPD (tunable spd/accel above), 0 = plain SET_POS (motor default profile)

// Soft re-engage: on disarmed→armed, approach the stick target gently
// (low spd/accel) instead of jumping straight to full power. Once close
// enough (or after a timeout, as a safety fallback), switch to normal
// full-power tracking above. Same scheme as EXPERIMENTAL.
#define AK60_CATCHUP_SPD        500    // electrical speed cap while catching up — tune by feel
#define AK60_CATCHUP_ACCEL      2000   // electrical accel cap while catching up — tune by feel
#define AK60_CATCHUP_DONE_DEG   2.0f   // catch-up ends once within this many degrees of target
#define AK60_CATCHUP_TIMEOUT_MS 3000UL // safety fallback if the threshold is never reached

static bool ak60_wasEngaged  = false;
static bool ak60_catchingUp  = false;
static unsigned long ak60_catchupT0 = 0;

static int joystickToSigned(int raw) {
    int val = raw - 127;
    if (val > -DEADBAND && val < DEADBAND) val = 0;
    return val;
}

static int toHB25(int val) {
    int pulse = HB25_STOP + (val * HB25_RANGE) / 127;
    if (pulse < HB25_MAX_REV) pulse = HB25_MAX_REV;
    if (pulse > HB25_MAX_FWD) pulse = HB25_MAX_FWD;
    return pulse;
}

static void stopAllMotors() {
    servos.writeServoPulse(CH_FL, HB25_STOP, true);
    servos.writeServoPulse(CH_FR, HB25_STOP, true);
    servos.writeServoPulse(CH_BL, HB25_STOP, true);
    servos.writeServoPulse(CH_BR, HB25_STOP, true);
}

static void omniDrive(int Y, int X, int R) {
    int fl =  Y + X + R;
    int fr =  Y - X - R;
    int bl =  -Y - X + R;
    int br =  -Y + X - R;

    // Scale down proportionally if any output exceeds range,
    // preserving direction of travel at full stick deflection
    int maxVal = max(max(abs(fl), abs(fr)), max(abs(bl), abs(br)));
    if (maxVal > 127) {
        fl = fl * 127 / maxVal;
        fr = fr * 127 / maxVal;
        bl = bl * 127 / maxVal;
        br = br * 127 / maxVal;
    }

    servos.writeServoPulse(CH_FL, toHB25(fl), true);
    servos.writeServoPulse(CH_FR, toHB25(fr), true);
    servos.writeServoPulse(CH_BL, toHB25(bl), true);
    servos.writeServoPulse(CH_BR, toHB25(br), true);
}

void platformSetup() {
    configureMotors();
    stopAllMotors();
    servos.writeServoPulse(CH_DOOR, HB25_STOP, true);
}

void platformLoop() {
    // ── Deferred CAN init — once, after all resources allocated ──────────────
    ak60Begin();

    int Y = joystickToSigned(channels[CRSF_CH_AXIS_Y2]);
    int X = joystickToSigned(channels[CRSF_CH_AXIS_X1]);
    int R = joystickToSigned(channels[CRSF_CH_AXIS_Y1]);
    omniDrive(Y, X, R);

    int doorPulse = map(channels[CRSF_CH_AXIS_X2], 0, 255, 1000, 2000);
    servos.writeServoPulse(CH_DOOR, doorPulse, true);

    // AK60 (mouth) — same channel as the door servo, operating in tandem for now.
    // Gated by SA/ARM: disarmed = backdrivable (0A), armed = position control,
    // with a soft catch-up ramp on re-engage — same scheme as EXPERIMENTAL.
    if (ak60Ready()) {
        float ak60Target = map(channels[CRSF_CH_AXIS_X2], 0, 255,
                                (int)(-AK60_POS_RANGE_DEG * 10), (int)(AK60_POS_RANGE_DEG * 10)) / 10.0f;
        bool engaged = channels[CRSF_CH_ARM] > 128;

        if (!engaged) {
            ak60SetCurrent(0.0f);
            ak60_wasEngaged = false;
        } else {
            if (!ak60_wasEngaged) {
                // Rising edge (just re-engaged) — start the soft catch-up phase.
                ak60_catchingUp = true;
                ak60_catchupT0  = millis();
            }
            if (ak60_catchingUp) {
                float gap = ak60Target - ak60Pos();
                if (gap < 0) gap = -gap;
                if (gap < AK60_CATCHUP_DONE_DEG ||
                    millis() - ak60_catchupT0 > AK60_CATCHUP_TIMEOUT_MS) {
                    ak60_catchingUp = false;
                }
            }
            if (ak60_catchingUp) {
                // Soft approach — low speed/accel until close to the stick target.
                ak60SetPosSpd(ak60Target, AK60_CATCHUP_SPD, AK60_CATCHUP_ACCEL);
            } else {
                // Speed/accel-limited move — the motor's own trajectory generator
                // eases into position, following the live stick target.
#if AK60_USE_POS_SPD
                ak60SetPosSpd(ak60Target, AK60_SPD_LIMIT, AK60_ACCEL_LIMIT);
#else
                ak60SetPos(ak60Target);
#endif
            }
            ak60_wasEngaged = true;
        }
        ak60DebugPrint();
    }

    servos.writeServoPulse(CH_KNOB, map(channels[5], 0, 255, 1000, 2000), true);

    extern bool brakeState;
    extern unsigned long brakeTimer;
    trommel.setSpeed(map(channels[6], 0, 255, 0, 255), brakeState);
}

void platformScreen() {}

void platformOnIdle() {
    stopAllMotors();
    // Door servo intentionally not reset — retains last position
}

#endif // OMNI_DRIVE

void platformSetup1()    {}
void platformLoopCore1() {}

#endif // WASHMACHINE