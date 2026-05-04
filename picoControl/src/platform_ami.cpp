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
// Sample list on disk:
/*
     Sample 1: fluitje         Sample 7: arm uit
     Sample 2: fietfiew         Sample 8: arm in
     Sample 3: kloinkskinkeldekikel  Sample 9: motorkap
     Sample 4: ratelratel       Sample 10: achterklep omhoog
     Sample 5:                  Sample 11: snurk
     Sample 6:                  Sample 12: deur
                                Sample 13: fly up
                                Sample 14: fly down
                                Sample 15: grill
                                Sample 16: knipoog
                                Sample 17: rook
                                Sample 18: zwaailamp
*/


Action myActionList[NUM_ACTIONS] = {
    Action(SW(1,1),   -1, DIRECT, nullptr, 100,  1, &player1), // [0]  track 1
    Action(SW(2,1),   -1, DIRECT, nullptr, 100,  2, &player1), // [1]  track 2
    Action(SW(3,1),   -1, DIRECT, nullptr, 100,  4, &player1), // [2]  track 3
    Action(SW(0,1),   11, DIRECT, nullptr, 100, 19, &player2), // [3]  zwaailicht    — relay 11
    Action(SW(5,1),    4, DIRECT, nullptr, 100, 10, &player2), // [4]  achterklep op — relay 4
    Action(SW(5,2),    5, DIRECT),                             // [5]  achterklep dicht — relay 5
    Action(SW(8,1),    0, DIRECT, nullptr, 100,  8, &player2), // [6]  arm uit       — relay 0
    Action(SW(8,2),    1, DIRECT, nullptr, 100,  7, &player2), // [7]  arm in        — relay 1
    Action(SW(6,1),   22, DIRECT, nullptr, 100,  9, &player2), // [8]  motorkap open — relay 22 (GPIO)
    Action(SW(6,2),   23, DIRECT),                             // [9]  motorkap dicht — relay 23 (GPIO)
    Action(SW(7,1),   14, DIRECT, nullptr, 100, 14, &player2), // [10] lift up        — relay 14
    Action(SW(7,2),   21, DIRECT, nullptr, 100, 15, &player2), // [11] elevator release — relay 21 (GPIO)
    Action(KEY_ACTION(KEY_BIT_1), 20, DIRECT),                 // [12] elevator release back — relay 20 (GPIO)
    Action(SW(4,1),   15, DIRECT, nullptr, 100, 13, &player2), // [13] vleugeldeur   — relay 15
    Action(SW(4,2),   12, DIRECT, nullptr, 100, 13, &player2), // [14] vleugeldeur terug — relay 12
    Action(-1,         2, DIRECT),                             // [15] hoofd (internal)
    Action(-1,         8, TRIGGER, nullptr, 100, 16, &player2),// [16] grill (internal)
    Action(KEY_ACTION(KEY_BIT_HASH), 16, DIRECT, nullptr, 100, 18, &player2), // [17] rook — relay 19 (GPIO)
    Action(KEY_ACTION(KEY_BIT_3),   18, DIRECT),               // [18] toet2 — relay 18 (GPIO)
    Action(KEY_ACTION(KEY_BIT_6),   17, DIRECT, nullptr, 100, 10, &player2),  // [19] bellen — relay 17 (GPIO)
    Action(-1,        13, DIRECT),                             // [20] adem (internal)
    Action(-1,        14, DIRECT),                             // [21] spare
    Action(KEY_ACTION(KEY_BIT_7),   -1, DIRECT, nullptr, 100,  4, &player2),  // [22]
    Action(KEY_ACTION(KEY_BIT_9),   -1, DIRECT, nullptr, 100,  5, &player2),  // [23]
    Action(KEY_ACTION(KEY_BIT_4),   19, DIRECT),               // [24] toet1 — relay 18 (GPIO)
};
// ----------------------------------------------------------------------------
// Sequence: looking animation
// ----------------------------------------------------------------------------
ActionSequence looking(SW(12,1), DIRECT, true);  // mux 12 = 3-pos switch

static bool blink = false;

static void configureSequences() {
    looking.addEvent(0,     EVENT_START, &myActionList[15]); // hoofd
    looking.addEvent(1,     EVENT_START, &myActionList[20]); // adem
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
    drainAudioQueue();   // Core 0: execute queued play/pause for both players
    looking.update();

    extern int channels[];
    extern bool getRemoteKey(int bit);

    // Eye blink on '*' button — eyelid command gets its own tick, no delay needed
    extern int channels[];
    extern bool getRemoteKey(int bit);
    static bool blinkPending = false;
    static bool blinkClosePending = false;
    if (getRemoteKey(KEY_BIT_STAR) && !blink) {
        blink = true;
        blinkPending = true;
        audioQueue.enqueue(AUDIO_PLAY, 2, 17);
    } else if (!getRemoteKey(KEY_BIT_STAR) && blink) {
        blink = false;
        blinkClosePending = true;
    }

    // Eye / eyelid servo via RS485 — only send when value changes
    // On blink tick, skip eye servo so eyelid command goes through alone
    {
        static int lastEye = -1;
        int eyeVal = map(channels[0], 255, 0, 50, 130);
        if (blinkPending) {
            RS485WriteByte(10, 2, 0);
            blinkPending = false;
            lastEye = -1;
        } else if (blinkClosePending) {
            RS485WriteByte(10, 2, 90);
            blinkClosePending = false;
            lastEye = -1;
        } else if (eyeVal != lastEye) {
            RS485WriteByte(10, 0, eyeVal);
            RS485WriteByte(10, 1, eyeVal);
            lastEye = eyeVal;
        }
    }
}

void platformScreen() {
    if (looking.isPlaying()) display.print(F("look"));
}

void platformOnIdle() {
    RS485WriteByte(18, 2, 0);   // motor neutral
    RS485WriteByte(22, 1, 127); // steer neutral
}


// Core 1: only CRSF (handled by crsfCore1Loop) and audio queue draining
void platformSetup1()    {}
void platformLoopCore1() {}   // Core 1: CRSF only

#endif // AMI
