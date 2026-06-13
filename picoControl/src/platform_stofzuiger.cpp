#ifdef STOFZUIGER

// ============================================================================
// platform_stofzuiger.cpp — Vacuum cleaner vehicle
//
// Controller: BetaFPV (no nunchuck)
//
// Channel mapping (BetaFPV standard layout):
//   channels[0] CRSF_CH_AXIS_X1 = right stick X → drive left/right (cross-mix)
//   channels[1] CRSF_CH_AXIS_Y1 = right stick Y → drive forward/back (cross-mix)
//   channels[2] CRSF_CH_AXIS_X2 = left  stick X → (spare)
//   channels[3] CRSF_CH_AXIS_Y2 = left  stick Y → DDSM210 spin speed
//
// Drive: cross-mix handled by main.cpp USE_MOTOR path (channels[0] and [1]).
//   No USE_SPEEDSCALING — BetaFPV, no nunchuck, drive always active when armed.
//
// DDSM210: SoftwareSerial on GP14 (TX) / GP15 (RX), 115200 baud.
//   Fully owned by this platform file.
// ============================================================================

#include "config.h"
#include "platform.h"
#include "Motor.h"
#include "ddsm_ctrl.h"
#include <SoftwareSerial.h>
#include "M5Unit8Servos.h"

extern M5Unit8Servos servos;  // declared in main.cpp

// saveValues — channel defaults when RF link is lost (127=centre for axes)
const int saveValues[] = { 127, 127,   0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//                              X1   Y1  X2   Y2
//                                   ^mouth=0→1150µs(closed)

// ----------------------------------------------------------------------------
// DDSM210 — fully owned here
// ----------------------------------------------------------------------------
DDSM_CTRL dc;
SoftwareSerial DDSMport(15, 14);   // RX=GP15, TX=GP14

#define DDSM_ID       4
#define DDSM_MAX_RPM  2100
#define DEADBAND      15    // ~12% of full range either side of centre

// ----------------------------------------------------------------------------
// Drive motors — Electromen H-bridge, board 3.5
// ----------------------------------------------------------------------------
Motor motorRight(18, 19, 20, -1);
Motor motorLeft (21, 22, 26, -1);

void configureMotors() {
    motorLeft.init();
    motorRight.init();
}

// ----------------------------------------------------------------------------
// Platform interface
// ----------------------------------------------------------------------------

void platformSetup() {
    configureMotors();

    // DDSM210 init — retry mode set to handle startup timing issues.
    // The DDSM may have been left in position/open loop from a previous session,
    // or may not be ready immediately. Send mode + zero command multiple times.
    DDSMport.begin(115200);
    dc.pSerial = &DDSMport;
    dc.set_ddsm_type(210);

    // Small delay to let DDSM power up and settle before first UART command
    delay(200);

    // Clear any garbage in the buffer from power-up noise
    dc.clear_ddsm_buffer();

    // Send mode switch + zero command 3 times to ensure it takes effect.
    // No feedback available from ddsm_change_mode, so retry is the only option.
    for (int i = 0; i < 3; i++) {
        dc.ddsm_change_mode(DDSM_ID, 2);   // speed loop
        delay(20);
        dc.ddsm_ctrl(DDSM_ID, 0, 0);      // zero speed
        delay(20);
    }
    dc.clear_ddsm_buffer();                // clear any feedback from the above

    // Mouth servo — attach and move to default closed position
    servos.writeServoPulse(0, 1150, true);  // 1150µs = closed
}

void platformLoop() {
    extern int channels[];

    // ── DDSM210 — left stick Y (channels[3]) ─────────────────────────────────
    // Periodic mode re-assertion — every 5 seconds re-send speed loop mode
    // to recover from any corrupted init or mode drift
    static unsigned long lastModeCheck = 0;
    if (millis() - lastModeCheck > 5000) {
        dc.ddsm_change_mode(DDSM_ID, 2);
        lastModeCheck = millis();
    }

    int yRaw = channels[CRSF_CH_AXIS_Y2] - 127;
    if (yRaw > -DEADBAND && yRaw < DEADBAND) yRaw = 0;
    int rpm = map(yRaw, -127, 127, -DDSM_MAX_RPM, DDSM_MAX_RPM);
    dc.ddsm_ctrl(DDSM_ID, rpm, 0);

    // ── Mouth servo — left stick X (channels[2]), M5Unit8Servos ch0 ─────────────
    // 1150µs = closed (default), 800µs = fully open
    int mouthPulse = map(channels[CRSF_CH_AXIS_X2], 0, 255, 1150, 800);
    servos.writeServoPulse(0, mouthPulse, true);
}

void platformScreen() {}

void platformOnIdle() {
    dc.ddsm_ctrl(DDSM_ID, 0, 0);
}

void platformSetup1()    {}
void platformLoopCore1() {}

#endif // STOFZUIGER