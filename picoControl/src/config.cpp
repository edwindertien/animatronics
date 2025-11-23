#include "vehicle_select.h"
#include "config.h"
#include "Audio.h"   // for player1, etc.
#include "Motor.h"
// Only build this when AMI is selected
#ifdef AMI
// IMPORTANT: this is now the single, real definition of myActionList
Action myActionList[NUM_ACTIONS] = {
  Action(2,  -1, DIRECT, nullptr, 100, 1, &player1), // track 1
  Action(4,  -1, DIRECT, nullptr, 100, 2, &player1), // track 2
  Action(6,  -1, DIRECT, nullptr, 100, 4, &player1), // track 3
  Action(0,  11, DIRECT),                            // zwaailicht
  Action(16, 4,  DIRECT),                            // achterklep open
  Action(17, 5,  DIRECT),                            // achterklep dicht
  Action(12, 0,  DIRECT, nullptr, 100, 7, &player2), // arm uit 
  Action(13, 1,  DIRECT),                            // arm in
  Action(10, 22, DIRECT),                            // motorkap open
  Action(11, 23, DIRECT),                            // motorkap dicht
  Action(14, 14, DIRECT, nullptr, 100, 13, &player2),// lift up
  Action(15, 21, DIRECT, nullptr, 100, 14, &player2),// elevator release
  Action('0',20, DIRECT),                            // elevator release back
  Action(8,  15, DIRECT),                            // vleugeldeur
  Action(9,  12, DIRECT),
  Action('7',18, DIRECT),                            // check-> hoofd
  Action('8',19, TRIGGER, nullptr, 100, 15, &player2), // check-> grill
  // (commented-out alternatives kept as reference in your original)
};

// And this is the single, real definition of the sequence
ActionSequence looking('5', TOGGLE, true);   // button '5', toggle, loop

// Sequence wiring for AMI
void configureSequences() {
    looking.addEvent(0,   EVENT_START, &myActionList[15]); // hoofd
    looking.addEvent(5000, EVENT_STOP,  &myActionList[15]); // hoofd
    looking.addEvent(5001, EVENT_START,  &myActionList[16]); // grill
    looking.addEvent(8009, EVENT_STOP,  &myActionList[16]); // grill
    looking.addEvent(12000, EVENT_START, &myActionList[15]); // hoofd
    looking.addEvent(15000, EVENT_STOP, &myActionList[15]); // hoofd
    looking.addEvent(20000, EVENT_STOP, &myActionList[15]); // hoofd
}
#endif // AMI


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
};
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

/////// Electromen motor drivers (or other sigh-magnitude PWM drivers)
Motor motorLeft(18, 19, 20, 1);   // Motor 1 (Pins 18, 19 for direction and 20 for PWM)
Motor motorRight(26, 27, 28, 2);  //
Motor tandkrans(21, 22, -1, -1);  // 26 and 27 for control, no PWM (motorcontroller set at fixed speed)

void configureMotors(){
  motorLeft.init();
  motorRight.init();
  tandkrans.init();
}
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

// left pin, right pin, pwm pin, brake relay pin. set unused pins to -1
Motor motorLeft(20, 19, 18, 0);   // Motor 1 (Pins 20, 19 for direction and 18 for PWM)
Motor motorRight(28, 27, 26, 1);  //
//Motor tandkrans(21, 22, -1, -1);  // 21 and 22 for control, no PWM (motorcontroller set at fixed speed)
void configureMotors(){
  motorLeft.init();
  motorRight.init();
  //tandkrans.init();
}
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
// left pin, right pin, pwm pin, brake relay pin. set unused pins to -1
Motor motorLeft(20, 19, 18, 0);   // Motor 1 (Pins 20, 19 for direction and 18 for PWM)
Motor motorRight(28, 27, 26, 1);  //
//Motor tandkrans(21, 22, -1, -1);  // 21 and 22 for control, no PWM (motorcontroller set at fixed speed)

void configureMotors(){
  motorLeft.init();
  motorRight.init();
  //tandkrans.init();
}
#endif