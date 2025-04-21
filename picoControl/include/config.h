//#define ANIMAL_LOVE
//#define KLARA
//#define ANIMALTRONIEK_KREEFT (1)
#define ANIMALTRONIEK_VIS (1)
//#define DEBUG (1)

////// hardware specifics for animaltroniek
#ifdef ANIMALTRONIEK_KREEFT
// for using DC motors, for example Electromen drives
// animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// two inputs for direction and 1 input for velocity.
#define USE_MOTOR (1)
#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60  // used for scaling in the cross-mix function
#define HIGH_SPEED 90 
// when no scaling used:
#define MAX_SPEED 50

// for serial output on the RJ45 socket
#define USE_RS485 (1)
// for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
#define USE_OLED (1)
// only use the encoder when these pins are not used for controlling separate motors
//#define USE_ENCODER (1)

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

// Actions can only be coupled with audio when the players are enabled
// note the board will only start when audio players are available
//#define USE_AUDIO (1)

#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 
#define NUM_CHANNELS 16
// at present 14 of the 16 channels are used. Enter the save values (FAILSAFE) in these arrays
//                                           0    1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
int channels[NUM_CHANNELS] =   { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//                                           X    Y nb kp vo sw sw sw sw
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

#define ANIMATION_KEY (32)
#endif





#ifdef ANIMALTRONIEK_VIS
// for using DC motors, for example Electromen drives
// animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// two inputs for direction and 1 input for velocity.
#define BOARD_V2 (1)
#define USE_MOTOR (1)
#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define USE_KEYPAD_SPEED (1) // use '#' for speed mode
#define LOW_SPEED 60  // used for scaling in the cross-mix function
#define HIGH_SPEED 80 
// when no scaling used:
#define MAX_SPEED 50

// for serial output on the RJ45 socket
#define USE_RS485 (1)
// for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
#define USE_OLED (1)
// only use the encoder when these pins are not used for controlling separate motors
//#define USE_ENCODER (1)

#define RELAY_POWER_1 11
#define RELAY_POWER_2 26

// important mapping of actions, buttons, relay channels and sounds
#define NUM_ACTIONS 4
Action myActionList[NUM_ACTIONS] = {
  //Action('a', -1, DIRECT, &tandkrans, 100, "/bubble.mp3", &player1),
  //Action('1', -1, DIRECT, &tandkrans, -100),
  //Action('2', -1, DIRECT, &tandkrans, -100),
  Action('3', 3, DIRECT),
  Action('4', 4, DIRECT),
  Action(10, 4, DIRECT), // on button s
  Action(11, 5, DIRECT),
};

// Actions can only be coupled with audio when the players are enabled
// note the board will only start when audio players are available
//#define USE_AUDIO (1)

#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 
#define NUM_CHANNELS 16
// at present 14 of the 16 channels are used. Enter the save values (FAILSAFE) in these arrays
//                                           0    1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
int channels[NUM_CHANNELS] =   { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//                                           X    Y nb kp vo sw sw sw sw
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

#define ANIMATION_KEY (32)
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