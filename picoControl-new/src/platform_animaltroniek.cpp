#if defined(ANIMALTRONIEK_VIS) || defined(ANIMALTRONIEK_KREEFT) || defined(ANIMALTRONIEK_SCHILDPAD)

#include "config.h"
#include "platform.h"
#include "Motor.h"

const int saveValues[] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ----------------------------------------------------------------------------
// Motor instances (shared pin layout between VIS and KREEFT)
// ----------------------------------------------------------------------------
Motor motorLeft (20, 19, 18, 0);
Motor motorRight(28, 27, 26, 1);

void configureMotors() {
    motorLeft.init();
    motorRight.init();
}

// ----------------------------------------------------------------------------
// Action list
// ----------------------------------------------------------------------------
Action myActionList[NUM_ACTIONS] = {
    Action(10, 4, DIRECT),
    Action(11, 5, DIRECT),
};

// ----------------------------------------------------------------------------
// Platform interface
// ----------------------------------------------------------------------------
void platformSetup() {
    configureMotors();
}

void platformLoop() {
    // Motor driving is handled in the shared USE_MOTOR block in main.cpp
}

void platformScreen() {
    // No sequence status for animaltroniek
}

void platformOnIdle() {
    // Motors are stopped by the shared USE_MOTOR brake logic in main.cpp
}


// Core 1 not used by this vehicle
void platformSetup1()    {}
void platformLoopCore1() {}

#endif // ANIMALTRONIEK_VIS || ANIMALTRONIEK_KREEFT || ANIMALTRONIEK_SCHILDPAD