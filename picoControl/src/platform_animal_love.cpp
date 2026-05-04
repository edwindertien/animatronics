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
    Action(SW(0,1),  -1, DIRECT, &tandkrans,  100), // [0] krans forward
    Action(SW(0,2),  -1, DIRECT, &tandkrans, -100), // [1] krans reverse
    Action(SW(9,1),   0, DIRECT),                   // [2] poten
    Action(SW(1,1),   3, DIRECT),                   // [3] lift
    Action(SW(1,2),   4, DIRECT),                   // [4] lift
    Action(SW(3,2),   5, DIRECT),                   // [5] ratel
    Action(SW(2,1),   7, DIRECT),                   // [6] sirene
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
