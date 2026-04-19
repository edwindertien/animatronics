#ifdef SCUBA

#include "config.h"
#include "platform.h"
#include "Audio.h"
#include "RS485Reader.h"

const int saveValues[] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ----------------------------------------------------------------------------
// Action list
// ----------------------------------------------------------------------------
Action myActionList[NUM_ACTIONS] = {
    Action('5', -1, DIRECT, nullptr, 100, 1, &player1), // [0]  bubble track (background)
    Action('8', -1, DIRECT, nullptr, 100, 2, &player1), // [1]  jaws track
    Action(0,   14, DIRECT),                            // [2]  hoofd
    Action(2,    1, DIRECT),                            // [3]  snorkel
    Action(3,    2, DIRECT),                            // [4]  snorkel
    Action(5,    3, DIRECT),                            // [5]  spuit
    Action(6,    4, DIRECT),                            // [6]  motorkap open valve
    Action(7,    5, DIRECT),                            // [7]  motorkap close valve
    Action('1', 10, DIRECT),                            // [8]  toeter
    Action('3',  7, DIRECT),                            // [9]  toeter
    Action('#',  8, DIRECT),                            // [10] bellen
    Action('0',  9, DIRECT),                            // [11]
    Action('*', 15, DIRECT),                            // [12] rook
};

// ----------------------------------------------------------------------------
// Sequence: jaws animation
// ----------------------------------------------------------------------------
ActionSequence jaws(4, TRIGGER, false);

static void configureSequences() {
    jaws.addEvent(0,      EVENT_START, &myActionList[1]);  // start jaws audio
    jaws.addEvent(17500,  EVENT_START, &myActionList[12]); // smoke ON
    jaws.addEvent(20000,  EVENT_STOP,  &myActionList[12]); // smoke OFF
    jaws.addEvent(20000,  EVENT_START, &myActionList[6]);  // hood OPEN valve ON
    jaws.addEvent(23500,  EVENT_STOP,  &myActionList[6]);  // hood OPEN valve OFF
    jaws.addEvent(23500,  EVENT_START, &myActionList[7]);  // hood CLOSE valve ON
    jaws.addEvent(24900,  EVENT_STOP,  &myActionList[7]);  // hood CLOSE valve OFF
    jaws.addEvent(24900,  EVENT_STOP,  &myActionList[1]);  // stop jaws audio → triggers bg resume
}

// ----------------------------------------------------------------------------
// Platform interface
// ----------------------------------------------------------------------------
void platformSetup() {
    RS485WriteByte(18, 1, 1);   // motor active
    delay(300);
    RS485WriteByte(18, 2, 0);   // motor neutral
    RS485WriteByte(22, 1, 127); // steer neutral
    configureSequences();
}

void platformLoop() {
    jaws.update();
    processBackground();
}

void platformScreen() {
    if (jaws.isPlaying()) display.print(F("jaws"));
}

void platformOnIdle() {
    RS485WriteByte(18, 2, 0);   // motor neutral
    RS485WriteByte(22, 1, 127); // steer neutral
}


// Core 1 not used by this vehicle
void platformSetup1()    {}
void platformLoopCore1() {}

#endif // SCUBA
