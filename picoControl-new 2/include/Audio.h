#ifndef AUDIO_H
#define AUDIO_H
#include <Arduino.h>
#include <DFRobot_DF1201S.h>    // always needed — Action.h uses this type
#include "SoftwareSerial.h"     // always needed — same reason

#if USE_AUDIO >= 1
extern DFRobot_DF1201S player1;
extern SoftwareSerial player1port;
void audioInit();
void processBackground();       // call once per loop tick — restarts bg track if needed
#ifdef BACKGROUND_TRACK_1
void scheduleBgResume1();       // call from Action::stop() when player1 finishes foreground
#endif
#endif

#if USE_AUDIO >= 2
extern DFRobot_DF1201S player2;
extern SoftwareSerial player2port;
#ifdef BACKGROUND_TRACK_2
void scheduleBgResume2();
#endif
#endif

#endif