#ifndef ACTION_H
#define ACTION_H

#include <Arduino.h>
#include "Adafruit_PWMServoDriver.h"
#include "DFRobot_DF1201S.h"
#include "Audio.h"


#define DIRECT 0
#define TOGGLE 1
#define TRIGGER 2

class Action {

  private:
    DFRobot_DF1201S* player;
    char button;
    int relay;
    int mode;
    int state;
    int previousState;
    const char *soundfile;

  public:
    Action(char button, int relay, int mode, const char *soundfile, DFRobot_DF1201S* player);
    void init();
    void update();
    void trigger();
    void stop();
    int getState();
};
#endif