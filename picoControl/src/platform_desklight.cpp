#ifdef DESKLIGHT

// ============================================================================
// platform_desklight.cpp — Desk light robot arm
//
// Hardware: 5x STS3215 serial bus servos (Waveshare/Feetech)
//           SoftwareSerial on GP12 (RX) / GP13 (TX) — BOARD V3.0
//           Controlled via SMS_STS library (SCServo)
//
// STS servo IDs and channel mapping:
//   ID 16 — channels[0] AXIS_X1  — pan/yaw         (pos 1024-3072)
//   ID 15 — channels[1] AXIS_Y1  — tilt/pitch      (pos 512-2048)
//   ID 14 — channels[2] AXIS_X2  — arm extend      (pos 2700-1024)
//   ID 12/13 — coupled pair from channels[2]        (mirrored around 1875)
//   ID 11 — channels[3] AXIS_Y2  — arm rotate      (pos 1024-3072)
//   ID 20 — channels[5] ANALOG1  — aux position    (pos 0-1048, speed from channels[6])
//
// saveValues: channels[0]=127, channels[1]=0, channels[3]=127 → centred/rest
// ============================================================================

#include "config.h"
#include "platform.h"
#ifdef USE_STS
#include <SCServo.h>
#endif

extern SMS_STS st;
extern int channels[];

// saveValues — channel defaults on link loss
// channels[0]=127 pan centre, channels[1]=0 tilt rest, channels[3]=127 rotate centre
const int saveValues[] = { 127, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ----------------------------------------------------------------------------
// Platform interface
// ----------------------------------------------------------------------------

void platformSetup() {
    // STSport and st initialised in main.cpp setup() under USE_STS:
    //   STSport.begin(1000000);
    //   st.pSerial = &STSport;
}

void platformLoop() {
    // SD (channels[7]): master off switch — 252=park/off, 3=normal
    // When SD is on, send failsafe positions to all servos and return
    if (channels[7] > 128) {
        st.WritePosEx(16, 2048, 2000, 100);  // pan centre
        st.WritePosEx(15,  512, 2000, 100);  // tilt down (head down)
        st.WritePosEx(14, 2700, 1000,  20);  // arm retracted
        st.WritePosEx(11, 2048, 1000,  20);  // rotate centre
        st.WritePosEx(20,    0, 1000, 100);  // lamp off

        st.WritePosEx(13, 1875 + 625 + 300, 1000, 20);
        st.WritePosEx(12, 1875 - 625,       1000, 20);

        return;
    }

    // SA (channels[4]): speed mode — 3=slow/smooth, 252=fast/responsive
    bool fastMode = (channels[4] > 128);

    // Speed and acceleration per mode:
    //   slow: speed=2000, accel=100  — smooth, cinematic movement
    //   fast: speed=4000, accel=254  — snappy, responsive (254=max accel for STS3215)
    int spd  = fastMode ? 4000 : 2000;
    int acc  = fastMode ? 254  : 100;
    int spd2 = fastMode ? 2000 : 1000;  // slower joints get proportionally scaled
    int acc2 = fastMode ? 254  : 20;

    // Pan/yaw
    st.WritePosEx(16, map(channels[0], 0, 255, 1024, 3072), spd, acc);

    // Light controller — SB(ch5) position, SC(ch6) speed (unchanged by SA)
    st.WritePosEx(20, map(channels[5], 0, 255, 0, 1048),
                      map(channels[6], 0, 255, 0, 2048), 100);

    // Tilt/pitch
    st.WritePosEx(15, map(channels[1], 0, 255, 512, 2048), spd, acc);

    // Arm extend — also drives coupled pair 12/13 mirrored around centre
    st.WritePosEx(14, map(channels[2], 0, 255, 2700, 1024), spd2, acc2);
    {
        int cv  = 1875;
        int val = map(channels[2], 0, 255, -625, 625);
        int off = 300;
        st.WritePosEx(13, cv - val + off, spd2, acc2);
        st.WritePosEx(12, cv + val,       spd2, acc2);
    }

    // Arm rotate
    st.WritePosEx(11, map(channels[3], 0, 255, 1024, 3072), spd2, acc2);
}

void platformScreen() {}

void platformOnIdle() {
    // STS3215 holds last commanded position on link loss — no explicit safe state needed
}

void platformSetup1()    {}
void platformLoopCore1() {}

#endif // DESKLIGHT