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
    Action(-1,  -1, DIRECT,  nullptr, 100, 1, &player1), // [0]  bubble track (background, sequence-triggered)
    Action(-1,  -1, TRIGGER, nullptr, 100, 2, &player1), // [1]  jaws track   (sequence-triggered)
    Action(SW(0, 1), 14, DIRECT),                        // [2]  hoofd        (sw0 high)
    Action(SW(1, 1),  1, DIRECT),                        // [3]  snorkel relay 1 (sw1 mid)
    Action(SW(1, 2),  2, DIRECT),                        // [4]  snorkel relay 2 (sw1 high)
    Action(KEY_ACTION(KEY_BIT_STAR),  3, DIRECT),        // [5]  spuit        (sw3 high)
    Action(SW(3, 1),  4, DIRECT),                        // [6]  motorkap open valve
    Action(SW(3, 2),  5, DIRECT),                        // [7]  motorkap close valve
    Action(KEY_ACTION(KEY_BIT_1), 10, DIRECT),           // [8]  toeter
    Action(KEY_ACTION(KEY_BIT_3),  7, DIRECT),           // [9]  toeter
    Action(KEY_ACTION(KEY_BIT_HASH), 8, DIRECT),         // [10] bellen
    Action(KEY_ACTION(KEY_BIT_0),  9, DIRECT),           // [11]
    Action(SW(2, 1), 15, DIRECT),                        // [12] rook
};

// ----------------------------------------------------------------------------
// Sequence: jaws animation
// ----------------------------------------------------------------------------
ActionSequence jaws(SW(2, 2), TRIGGER, false);

static void configureSequences() {
    jaws.addEvent(10,     EVENT_START, &myActionList[1]);  // start jaws audio
    jaws.addEvent(17500,  EVENT_START, &myActionList[12]); // smoke ON
    jaws.addEvent(20000,  EVENT_STOP,  &myActionList[12]); // smoke OFF
    jaws.addEvent(20000,  EVENT_START, &myActionList[6]);  // hood OPEN valve ON
    jaws.addEvent(23500,  EVENT_STOP,  &myActionList[6]);  // hood OPEN valve OFF
    jaws.addEvent(23500,  EVENT_START, &myActionList[7]);  // hood CLOSE valve ON
    jaws.addEvent(24900,  EVENT_STOP,  &myActionList[7]);  // hood CLOSE valve OFF
    jaws.addEvent(24900,  EVENT_STOP,  &myActionList[1]);  // stop jaws audio
}

// ----------------------------------------------------------------------------
// Platform interface
// ----------------------------------------------------------------------------
void platformSetup() {
    audioInit();            // blocking init on Core 0 — done before Core 1 starts
    RS485WriteByte(18, 1, 1);
    delay(300);
    RS485WriteByte(18, 2, 0);
    RS485WriteByte(22, 1, 127);
    configureSequences();
}

void platformLoop() {
    jaws.update();
    drainAudioQueue();    // Core 0: execute queued play/pause
    processBackground();  // Core 0: restart bubble track when needed
}

void platformScreen() {
    if (jaws.isPlaying()) display.print(F("jaws"));
}

void platformOnIdle() {
    RS485WriteByte(18, 2, 0);
    RS485WriteByte(22, 1, 127);
}

void platformSetup1()    {}   // Core 1: CRSF only
void platformLoopCore1() {}   // Core 1: CRSF only

#endif // SCUBA