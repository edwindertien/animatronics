#ifdef LUMI

#include "config.h"
#include "platform.h"
#include "Audio.h"
#include "PicoRelay.h"

extern PicoRelay relay;

const int saveValues[] = { 127, 127, 127, 127, 0, 127, 127, 0, 0, 127, 0, 0, 0, 0, 0, 0};

const String tracklist[NUM_TRACKS] = {
    "/mp3/01-int.mp3", "/mp3/02-dro.mp3", "/mp3/03-maz.mp3",
    "/mp3/04-sco.mp3", "/mp3/05-spi.mp3", "/mp3/06-pat.mp3",
    "/mp3/07-moe.mp3", "/mp3/08-wal.mp3", "/mp3/09-poo.mp3",
    "/mp3/10-cer.mp3", "/mp3/11-opt.mp3", "/mp3/12-kar.mp3",
    "/mp3/13-and.mp3", "/mp3/14-mid.mp3", "/mp3/15-ora.mp3",
};

const String samplelist[NUM_SAMPLES] = {
    "/mp3/01-alm.mp3", "/mp3/02-ang.mp3", "/mp3/03-slp.mp3",
    "/mp3/04-mov.mp3", "/mp3/05-noo.mp3", "/mp3/06-yes.mp3",
};

// ----------------------------------------------------------------------------
// Action list
// ----------------------------------------------------------------------------
Action myActionList[NUM_ACTIONS] = {
    Action(0, 10, DIRECT),
    Action(2,  6, DIRECT),
    Action(3,  7, DIRECT),
    Action(4,  9, DIRECT),
    Action(5,  8, DIRECT),
    Action(6, 11, DIRECT),
};

// ----------------------------------------------------------------------------
// Audio processing (Lumi has complex track/sample logic on player1 + player2)
// ----------------------------------------------------------------------------
static void processAudio() {
    extern int channels[];
    static int isPlaying    = 0;
    static int playingSample = 0;

    int trackToPlay = channels[13] / 8;
    if (trackToPlay == 0 && isPlaying && channels[6] < 100) {
        player1.pause();
        isPlaying = 0;
    } else if (trackToPlay > 0 && trackToPlay < (NUM_TRACKS + 1)
               && trackToPlay != isPlaying && channels[6] < 100) {
        player1.playSpecFile(tracklist[trackToPlay - 1]);
        isPlaying = trackToPlay;
    }

    static int volume1;
    if (channels[4] != volume1) { player1.setVol(map(channels[4], 0, 255, 0, 32)); volume1 = channels[4]; }

    static int volume2;
    if (channels[7] != volume2) { player2.setVol(map(channels[7], 0, 255, 0, 32)); volume2 = channels[7]; }

    if      (channels[9] > (127 + 30) && playingSample != 2)
        { player2.playFileNum(1); playingSample = 2; }
    else if (channels[9] < (127 - 30) && playingSample != 5)
        { player2.playFileNum(6); playingSample = 5; }
    else if ((channels[11] & 16) && playingSample != 3)
        { player2.playFileNum(3); playingSample = 3; }
    else if ((channels[11] & 64) && playingSample != 6)
        { player2.playFileNum(7); playingSample = 6; }
    else if (channels[0] < 100 && playingSample != 4)
        { player2.playFileNum(4); playingSample = 4; }
    else if (((channels[12] & 16) || (channels[12] & 32)) && playingSample != 1)
        { player2.playFileNum(8); playingSample = 1; }
}

// ----------------------------------------------------------------------------
// Platform interface
// ----------------------------------------------------------------------------
void platformSetup() {
    // Lumi audio is initialised on Core 1 (audioInit() called in setup1())
    // Nothing else needed here
}

void platformLoop() {
    extern int channels[];
    extern bool getRemoteSwitch(char);

    if (getRemoteSwitch(0)) relay.joystickToRelays(channels[0], channels[1]);
}

// Lumi audio runs on Core 1 — called from loop1()
void platformSetup1() {
    // Lumi initialises audio on Core 1 to keep SoftwareSerial off Core 0
    audioInit();
}

void platformLoopCore1() {
    drainAudioQueue();   // handle any action-triggered play/pause from Core 0
    processAudio();      // Lumi's channel-driven track/sample logic
}

void platformScreen() {
    // Lumi has no sequence status to show
}

void platformOnIdle() {
    // Lumi has no RS485 motors
}

#endif // LUMI