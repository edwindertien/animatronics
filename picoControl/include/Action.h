#ifndef ACTION_H
#define ACTION_H

#include <Arduino.h>
#include "Motor.h"
#include "Audio.h"

#define DIRECT 0
#define TOGGLE 1
#define TRIGGER 2
class Action {

  private:
  Motor* motor = nullptr;
  int motorvalue = 0;
  //const char* soundfile = nullptr;
  int tracknr = 0;
  int track = 0;
  DFRobot_DF1201S* player = nullptr;
    char button;
    int relaynr;
    int mode;
    int state;
    int previousState;
    void init();

  public:
  Action(char button, int relaynr, int mode);
  Action(char button, int relaynr, int mode, Motor* motor, int motorvalue);
  //Action(char button, int relay, int mode, Motor* motor, int motorvalue, const char *soundfile, DFRobot_DF1201S* player);
  Action(char button, int relaynr, int mode, Motor* motor, int motorvalue, int track, DFRobot_DF1201S* player);
       
    void update();
    void trigger();
    void stop();
    int getState();
};
#endif



