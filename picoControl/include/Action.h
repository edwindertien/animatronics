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
  Action(0, -1, DIRECT, &tandkrans, 100),
  Action(1, -1, DIRECT, &tandkrans, -100),
  Action(18, 0, DIRECT), // poten
  Action(2, 3, DIRECT), // lift
  Action(3, 4, DIRECT), // lift
  Action(7, 5, DIRECT), // ratel

// switch 0..7
// [0] krans
// [1] krans
// [2] lift
//  3 lift
//  5 water
//  6 blaas
//  7 ratel
// switch 8..15
// 8 L+R
// 10 bek 
// switch 16..23
// 18 poten
// switch 24..31
// 30 sequence

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

#ifdef AMI
// important mapping of actions, buttons, relay channels and sounds
#define NUM_ACTIONS 8
Action myActionList[NUM_ACTIONS] = {
  Action(18, -1, DIRECT, nullptr, 100, 1, &player1),
  Action(20, -1, DIRECT, nullptr, 100, 2, &player1),
  Action(22, -1, DIRECT, nullptr, 100, 4, &player1),
  Action('2', -1, TRIGGER, nullptr, 100, 1, &player2),
  Action('3', 2, DIRECT, nullptr, 100, 2, &player2),
  Action('4', 3, DIRECT, nullptr, 100, 3, &player2),
  Action('5', 4, DIRECT, nullptr, 100, 4, &player2),
  //Action('1', -1, DIRECT, &tandkrans, -100),
  //Action('2', -1, DIRECT, &tandkrans, -100),
  Action('1', 0, DIRECT)
  //Action('4', 4, DIRECT),
  //Action(10, 4, DIRECT), // on button s
  //Action(11, 5, DIRECT),
};
#endif