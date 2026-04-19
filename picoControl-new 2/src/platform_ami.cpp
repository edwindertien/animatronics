#ifdef AMI

#include "config.h"
#include "platform.h"
#include "Audio.h"
#include "RS485Reader.h"

const int saveValues[] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ----------------------------------------------------------------------------
// Action list
// ----------------------------------------------------------------------------
Action myActionList[NUM_ACTIONS] = {
    Action(2,   -1, DIRECT, nullptr, 100,  1, &player1), // [0]  track 1
    Action(4,   -1, DIRECT, nullptr, 100,  2, &player1), // [1]  track 2
    Action(6,   -1, DIRECT, nullptr, 100,  4, &player1), // [2]  track 3
    Action(0,   11, DIRECT, nullptr, 100, 18, &player2), // [3]  zwaailicht
    Action(10,   4, DIRECT, nullptr, 100, 10, &player2), // [4]  achterklep open
    Action(11,   5, DIRECT),                             // [5]  achterklep dicht
    Action(16,   0, DIRECT, nullptr, 100,  8, &player2), // [6]  arm uit
    Action(17,   1, DIRECT, nullptr, 100,  7, &player2), // [7]  arm in
    Action(12,  22, DIRECT, nullptr, 100,  9, &player2), // [8]  motorkap open
    Action(13,  23, DIRECT),                             // [9]  motorkap dicht
    Action(14,  14, DIRECT, nullptr, 100, 13, &player2), // [10] lift up
    Action(15,  21, DIRECT, nullptr, 100, 14, &player2), // [11] elevator release
    Action('1', 20, DIRECT),                             // [12] elevator release back
    Action(8,   15, DIRECT, nullptr, 100, 12, &player2), // [13] vleugeldeur
    Action(9,   12, DIRECT),                             // [14]
    Action('-',  2, DIRECT),                             // [15] hoofd
    Action('-',  8, TRIGGER, nullptr, 100, 15, &player2),// [16] grill
    Action('3', 19, DIRECT),                             // [17] toet
    Action('*', 16, DIRECT, nullptr, 100, 17, &player2), // [18] rook
    Action('4', 18, DIRECT),                             // [19] toet2
    Action('6', 17, DIRECT, nullptr, 100, 10, &player2), // [20] bellen
    Action('-', 13, DIRECT),                             // [21] adem
};

// ----------------------------------------------------------------------------
// Sequence: looking animation
// ----------------------------------------------------------------------------
ActionSequence looking(24, DIRECT, true);

static bool blink = false;

static void configureSequences() {
    looking.addEvent(0,     EVENT_START, &myActionList[15]); // hoofd
    looking.addEvent(1,     EVENT_START, &myActionList[21]); // adem
    looking.addEvent(5000,  EVENT_STOP,  &myActionList[15]); // hoofd stop
    looking.addEvent(5001,  EVENT_START, &myActionList[16]); // grill
    looking.addEvent(8009,  EVENT_STOP,  &myActionList[16]); // grill stop
    looking.addEvent(12000, EVENT_START, &myActionList[15]); // hoofd
    looking.addEvent(15000, EVENT_STOP,  &myActionList[15]); // hoofd stop
    looking.addEvent(20000, EVENT_STOP,  &myActionList[15]); // hoofd stop
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
    looking.update();

    // Eye blink on '#' button
    extern int channels[];
    extern bool getRemoteSwitch(char);
    if (getRemoteSwitch('#') && !blink) {
        blink = true;
        player2port.listen();
        player2.playFileNum(16);
        RS485WriteByte(10, 2, 0);
        delay(1);
    } else if (!getRemoteSwitch('#') && blink) {
        blink = false;
        RS485WriteByte(10, 2, 90);
        delay(1);
    }

    // Eye / eyelid servo via RS485
    RS485WriteByte(10, 0, map(channels[0], 255, 0, 50, 130));
    RS485WriteByte(10, 1, map(channels[0], 255, 0, 50, 130));
}

void platformScreen() {
    if (looking.isPlaying()) display.print(F("look"));
}

void platformOnIdle() {
    RS485WriteByte(18, 2, 0);   // motor neutral
    RS485WriteByte(22, 1, 127); // steer neutral
}


// Core 1 not used by this vehicle
void platformSetup1()    {}
void platformLoopCore1() {}

#endif // AMI
