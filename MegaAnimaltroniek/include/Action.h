#ifndef ACTION_H
#define ACTION_H

#include <Arduino.h>
//#include "Adafruit_PWMServoDriver.h"
//#include "Motor.h"
//#include "DFRobot_DF1201S.h"
//#include "Audio.h"


#define DIRECT 0
#define TOGGLE 1
#define TRIGGER 2

class Action {

  private:
  //Motor* motor = nullptr;
  //int motorvalue = 0;
  //const char* soundfile = nullptr;
  //DFRobot_DF1201S* player = nullptr;
    char button;
    int relay;
    int relay1;
    int relay2;
    int relay3;
    int mode;
    int state;
    int previousState;
    void init();

  public:
  Action(char button, int relay, int mode);
  Action(char button, int relay, int mode, int relay1);
  Action(char button, int relay, int mode, int relay1, int relay2);
  Action(char button, int relay, int mode, int relay1, int relay2, int relay3);
  //Action(char button, int relay, int mode, Motor* motor, int motorvalue);
  //Action(char button, int relay, int mode, Motor* motor, int motorvalue, const char *soundfile, DFRobot_DF1201S* player);
    
    void update();
    void trigger();
    void stop();
    int getState();
};
#endif