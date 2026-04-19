#include "config.h"
#include "Audio.h"

#if USE_AUDIO >= 1
void audioInit() {
    player1port.begin(115200);
    player1port.listen();
    while (!player1.begin(player1port)) {
        Serial.println("Player 1 init failed, check wiring!");
        delay(1000);
    }
    player1.switchFunction(player1.MUSIC);
    player1.setPrompt(false);
    player1.setPlayMode(player1.SINGLECYCLE);
    player1.setVol(0);
    player1.playFileNum(1);
    delay(10);
    player1.pause();
    delay(10);
    player1.playFileNum(1);
    delay(10);
    player1.pause();

#if USE_AUDIO >= 2
    player2port.begin(115200);
    player2port.listen();
    while (!player2.begin(player2port)) {
        Serial.println("Player 2 init failed, check wiring!");
        delay(1000);
    }
    player2.switchFunction(player2.MUSIC);
    player2.setPlayMode(player2.SINGLE);
    player2.setPrompt(false);
    player2.setVol(0);
    player2.playFileNum(1);
    player2.setVol(0);
    delay(10);
    player2.pause();
    delay(10);
    player2.playFileNum(1);
    player2.setVol(0);
    delay(10);
    player2.pause();
#endif // USE_AUDIO >= 2
}

// ----------------------------------------------------------------------------
// Background track management
// ----------------------------------------------------------------------------
#ifdef BACKGROUND_TRACK_1
static bool _bg1Active  = false;
static bool _bg1Pending = false;

void scheduleBgResume1() { _bg1Pending = true; }
#endif

#ifdef BACKGROUND_TRACK_2
static bool _bg2Active  = false;
static bool _bg2Pending = false;

void scheduleBgResume2() { _bg2Pending = true; }
#endif

void processBackground() {
#ifdef BACKGROUND_TRACK_1
    if (!_bg1Active) {
        player1port.listen();
        player1.setPlayMode(player1.SINGLECYCLE);
        player1.playFileNum(BACKGROUND_TRACK_1);
        _bg1Active  = true;
        _bg1Pending = false;
    } else if (_bg1Pending) {
        player1port.listen();
        player1.playFileNum(BACKGROUND_TRACK_1);
        _bg1Pending = false;
    }
#endif
#if USE_AUDIO >= 2
#ifdef BACKGROUND_TRACK_2
    if (!_bg2Active) {
        player2port.listen();
        player2.setPlayMode(player2.SINGLECYCLE);
        player2.playFileNum(BACKGROUND_TRACK_2);
        _bg2Active  = true;
        _bg2Pending = false;
    } else if (_bg2Pending) {
        player2port.listen();
        player2.playFileNum(BACKGROUND_TRACK_2);
        _bg2Pending = false;
    }
#endif
#endif
}

#endif // USE_AUDIO >= 1