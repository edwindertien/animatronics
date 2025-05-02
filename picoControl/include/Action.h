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
  const char* soundfile = nullptr;
  DFRobot_DF1201S* player = nullptr;
    char button;
    int relay;
    int mode;
    int state;
    int previousState;
    void init();

  public:
  Action(char button, int relay, int mode);
  Action(char button, int relay, int mode, Motor* motor, int motorvalue);
  Action(char button, int relay, int mode, Motor* motor, int motorvalue, const char *soundfile, DFRobot_DF1201S* player);
    
    void update();
    void trigger();
    void stop();
    int getState();
};
#endif


#ifdef LUMI
// important mapping of actions, buttons, relay channels and sounds
#define NUM_ACTIONS 3
Action myActionList[NUM_ACTIONS] = {
  Action('1', -1, DIRECT, nullptr, 100, "/sample1.mp3", &player2),
  //Action('1', -1, DIRECT, &tandkrans, -100),
  //Action('2', -1, DIRECT, &tandkrans, -100),
  //Action('3', 3, DIRECT),
  //Action('4', 4, DIRECT),
  Action(10, 4, DIRECT), // on button s
  Action(11, 5, DIRECT),
};
#endif

#ifdef ROBOT_LOVE
#define NUM_ACTIONS 6
Action myActionList[NUM_ACTIONS] = {
//  Action('a', -1, DIRECT, &tandkrans, 100, "/bubble.mp3", &player1),
  Action('1', -1, DIRECT, &tandkrans, -100),
  Action('2', -1, DIRECT, &tandkrans, -100),
  Action('3', 3, DIRECT),
  Action('4', 4, DIRECT),
  Action(10, 4, DIRECT),
  Action(11, 5, DIRECT),
};
#endif

#ifdef ANIMALTRONIEK_VIS
// important mapping of actions, buttons, relay channels and sounds
#define NUM_ACTIONS 2
Action myActionList[NUM_ACTIONS] = {
  //Action('a', -1, DIRECT, &tandkrans, 100, "/bubble.mp3", &player1),
  //Action('1', -1, DIRECT, &tandkrans, -100),
  //Action('2', -1, DIRECT, &tandkrans, -100),
  //Action('3', 3, DIRECT),
  //Action('4', 4, DIRECT),
  Action(10, 4, DIRECT), // on button s
  Action(11, 5, DIRECT),
};
#endif

#ifdef ANIMALTRONIEK_KREEFT
// important mapping of actions, buttons, relay channels and sounds
#define NUM_ACTIONS 2
Action myActionList[NUM_ACTIONS] = {
  //Action('a', -1, DIRECT, &tandkrans, 100, "/bubble.mp3", &player1),
  //Action('1', -1, DIRECT, &tandkrans, -100),
  //Action('2', -1, DIRECT, &tandkrans, -100),
  //Action('3', 3, DIRECT),
  //Action('4', 4, DIRECT),
  Action(10, 4, DIRECT), // on button s
  Action(11, 5, DIRECT),
};
#endif