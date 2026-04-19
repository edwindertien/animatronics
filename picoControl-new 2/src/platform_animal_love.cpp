#ifdef ANIMAL_LOVE

#include "config.h"
#include "platform.h"
#include "Motor.h"

const int saveValues[] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ----------------------------------------------------------------------------
// Motor instances
// ----------------------------------------------------------------------------
Motor motorLeft (18, 19, 20,  1);
Motor motorRight(26, 27, 28,  2);
Motor tandkrans (21, 22, -1, -1);

void configureMotors() {
    motorLeft.init();
    motorRight.init();
    tandkrans.init();
}

// ----------------------------------------------------------------------------
// Action list
// ----------------------------------------------------------------------------
Action myActionList[NUM_ACTIONS] = {
    Action(0,  -1, DIRECT, &tandkrans,  100), // [0] krans forward
    Action(1,  -1, DIRECT, &tandkrans, -100), // [1] krans reverse
    Action(18,  0, DIRECT),                   // [2] poten
    Action(2,   3, DIRECT),                   // [3] lift
    Action(3,   4, DIRECT),                   // [4] lift
    Action(7,   5, DIRECT),                   // [5] ratel
};

// ----------------------------------------------------------------------------
// Platform interface
// ----------------------------------------------------------------------------
void platformSetup() {
    configureMotors();
}

void platformLoop() {
    // Motor driving handled by shared USE_MOTOR block in main.cpp
}

void platformScreen() {
}

void platformOnIdle() {
}


// Core 1 not used by this vehicle
void platformSetup1()    {}
void platformLoopCore1() {}

#endif // ANIMAL_LOVE
