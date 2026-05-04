#ifdef STOFZUIGER

#include "config.h"
#include "platform.h"
#include "Motor.h"
#include "M5Unit8Servos.h"

const int saveValues[] = { 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ----------------------------------------------------------------------------
// Motor instances
// ----------------------------------------------------------------------------
Motor motorRight(18, 19, 20, -1);   // 
Motor motorLeft(21, 22, 26, -1);  //


void configureMotors() {
    motorLeft.init();
    motorRight.init();
}

// ----------------------------------------------------------------------------
// Platform interface
// ----------------------------------------------------------------------------
void platformSetup() {
    configureMotors();
}

void platformLoop() {
    extern int channels[];
    #ifdef USE_M5_SERVOS
    extern M5Unit8Servos servos;
    #endif
    extern bool brakeState;
    extern unsigned long brakeTimer;

#ifdef USE_M5_SERVOS
    if (channels[7] == 252) {
        servos.writeServoPulse(7, map(channels[2], 0, 255, 2000, 1300), true);
        servos.writeServoPulse(0, map(channels[6], 0, 255, 1000, 2000), true);
    } else {
        servos.detach(0);
        servos.detach(7);
    }
    #endif
}

void platformScreen() {
}

void platformOnIdle() {
}


// Core 1 not used by this vehicle
void platformSetup1()    {}
void platformLoopCore1() {}

#endif // WASHMACHINE