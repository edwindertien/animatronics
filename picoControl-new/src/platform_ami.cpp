#ifdef AMI

#include "config.h"
#include "platform.h"
#include "Audio.h"
#include "AudioQueue.h"
#include "RS485Reader.h"

const int saveValues[] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ----------------------------------------------------------------------------
// Action list
// ----------------------------------------------------------------------------
// AMI transmitter switch mapping (from switchChannel[] in picoRemote config.h):
// mux 0  = 3-pos switch
// mux 1  = 3-pos switch
// mux 2  = 3-pos switch
// mux 3  = unused
// mux 4  = 2-pos switch
// mux 5  = 2-pos switch
// mux 6  = 2-pos switch
// mux 7  = 2-pos switch
// mux 8-11 = unused
// mux 12 = 3-pos switch
// mux 13-15 = unused
//
// Relay board V1 (PCA9685): channels 0-15 only
// Actions using relays >15 require EXTRA_RELAY (GPIO) — verify hardware

Action myActionList[NUM_ACTIONS] = {
    Action(SW(1,1),   -1, DIRECT, nullptr, 100,  1, &player1), // [0]  track 1       (mux 4)
    Action(SW(2,1),   -1, DIRECT, nullptr, 100,  2, &player1), // [1]  track 2       (mux 5)
    Action(SW(3,1),   -1, DIRECT, nullptr, 100,  4, &player1), // [2]  track 3       (mux 6)
    Action(SW(0,1),   11, DIRECT, nullptr, 100, 19, &player2), // [3]  zwaailicht    (mux 0)
    Action(SW(5,1),    4, DIRECT, nullptr, 100, 10, &player2), // [4]  achterklep op (mux 7)
    Action(SW(5,2),   5, DIRECT),                             // [5]  achterklep dicht (mux 12)
    Action(SW(8,1),    0, DIRECT, nullptr, 100,  8, &player2), // [6]  arm uit       (mux 1)
    Action(SW(8,2),    1, DIRECT, nullptr, 100,  7, &player2), // [7]  arm in        (mux 2, mid)
    Action(SW(6,1),  6, DIRECT, nullptr, 100,  9, &player2), // [8]  motorkap open
    Action(SW(6,2),  7, DIRECT, nullptr, 100,  9, &player2),           // [9]  motorkap dicht
    Action(SW(7,1), 14, DIRECT, nullptr, 100, 14, &player2), // [10] lift up
    Action(SW(7,2), 15, DIRECT, nullptr, 100, 15, &player2), // [11] elevator release
    Action(KEY_ACTION(KEY_BIT_1), 13, DIRECT),           // [12] elevator release back
    Action(SW(4,1),  8, DIRECT, nullptr, 100, 13, &player2), // [13] vleugeldeur
    Action(SW(4,2),  9, DIRECT, nullptr, 100, 13, &player2),           // [14] vleugeldeur terug
    Action(-1,   2, DIRECT),                             // [15] hoofd (internal only)
    Action(-1,   3, TRIGGER, nullptr, 100, 16, &player2),// [16] grill (internal only)
    Action(KEY_ACTION(KEY_BIT_HASH), 10, DIRECT, nullptr, 100, 18, &player2), // [17] rook
    Action(KEY_ACTION(KEY_BIT_3),  12, DIRECT),          // [18] toet2
    Action(KEY_ACTION(KEY_BIT_6),  11, DIRECT, nullptr, 100, 10, &player2),   // [19] bellen
    Action(-1,  13, DIRECT),                             // [20] adem (internal only)
    Action(-1,  14, DIRECT),                             // [21] spare
    Action(KEY_ACTION(KEY_BIT_7),  -1, DIRECT, nullptr, 100, 4, &player2),   // [19] bellen
    Action(KEY_ACTION(KEY_BIT_9),  -1, DIRECT, nullptr, 100, 5, &player2),   // [19] bellen
};

// ----------------------------------------------------------------------------
// Sequence: looking animation
// ----------------------------------------------------------------------------
ActionSequence looking(SW(12,1), DIRECT, true);  // mux 12 = 3-pos switch

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
    audioInit();            // blocking init on Core 0 — done before Core 1 starts
    RS485WriteByte(18, 1, 1);
    delay(300);
    RS485WriteByte(18, 2, 0);
    RS485WriteByte(22, 1, 127);
    configureSequences();
}

void platformLoop() {
    looking.update();

    // Eye blink on '*' button
    extern int channels[];
    extern bool getRemoteKey(int bit);
    if (getRemoteKey(KEY_BIT_STAR) && !blink) {
        blink = true;
        audioQueue.enqueue(AUDIO_PLAY, 2, 17);  // eye blink sound on player2
        RS485WriteByte(10, 2, 0);
        delay(1);
    } else if (!getRemoteKey(KEY_BIT_STAR) && blink) {
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


// Core 1: only CRSF (handled by crsfCore1Loop) and audio queue draining
void platformSetup1()    {}   // audioInit is on Core 0 in platformSetup()
void platformLoopCore1() { drainAudioQueue(); }

#endif // AMI