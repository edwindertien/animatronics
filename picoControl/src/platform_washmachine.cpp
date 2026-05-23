#ifdef WASHMACHINE

#include "config.h"
#include "platform.h"
#include "Motor.h"
#include "M5Unit8Servos.h"

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

#define HB25_STOP     1500
#define HB25_MAX_FWD  2000
#define HB25_MAX_REV  1000
#define HB25_RANGE     500

#define CH_FL    0
#define CH_FR    1
#define CH_BL    2
#define CH_BR    3
#define CH_DOOR  6

#define DEADBAND 4

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
    int bl =  Y - X + R;
    int br =  Y + X - R;

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
    stopAllMotors();
    servos.writeServoPulse(CH_DOOR, HB25_STOP, true);
}

void platformLoop() {
    int Y = joystickToSigned(channels[CRSF_CH_AXIS_Y1]);
    int X = joystickToSigned(channels[CRSF_CH_AXIS_X1]);
    int R = joystickToSigned(channels[CRSF_CH_AXIS_X2]);
    omniDrive(Y, X, R);

    int doorPulse = map(channels[CRSF_CH_AXIS_Y2], 0, 255, 1000, 2000);
    servos.writeServoPulse(CH_DOOR, doorPulse, true);
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