#ifndef ACTION_H
#define ACTION_H

#include <Arduino.h>
#include "Motor.h"
#include "Audio.h"      // provides DFRobot_DF1201S type + conditional externs

#define DIRECT 0
#define TOGGLE 1
#define TRIGGER 2

class Action {
  private:
    Motor* motor = nullptr;
    int motorvalue = 0;
    int tracknr = 0;
    int track = 0;
#if USE_AUDIO >= 1
    DFRobot_DF1201S* player = nullptr;
#endif
    char button;
    int relaynr;
    int mode;
    int state;
    int previousState;
    void init();

  public:
    Action(char button, int relaynr, int mode);
    Action(char button, int relaynr, int mode, Motor* motor, int motorvalue);
#if USE_AUDIO >= 1
    Action(char button, int relaynr, int mode, Motor* motor, int motorvalue, int track, DFRobot_DF1201S* player);
#endif
    void update();
    void trigger();
    void stop();
    int getState();
};
#endif