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
#define NUM_ACTIONS 6
Action myActionList[NUM_ACTIONS] = {
  Action(0,10,DIRECT),
  Action(2,6,DIRECT),
  Action(3,7,DIRECT),
  Action(4,9,DIRECT),
  Action(5,8,DIRECT),
  Action(6,11,DIRECT)
  // if(channels[12]&1<<0)writeRelay(10,HIGH); else writeRelay(10,LOW);
  // if(channels[12]&1<<4)writeRelay(9,HIGH); else writeRelay(9,LOW);
  // if(channels[12]&1<<5)writeRelay(8,HIGH); else writeRelay(8,LOW);
  // if(channels[12]&1<<6)writeRelay(11,HIGH); else writeRelay(11,LOW);
  // if(channels[12]&1<<2)writeRelay(6,HIGH); else writeRelay(6,LOW); // wheels out
  // if(channels[12]&1<<3)writeRelay(7,HIGH); else writeRelay(7,LOW); // wheels in
};
#endif

#ifdef ANIMAL_LOVE
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