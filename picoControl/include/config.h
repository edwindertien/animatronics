//#define DEBUG (1)

// note that this definition has its consequences in the main.cpp and also in action.h (where
// each action mapping is given)
//////////////////////////////////////////////////////////////////////////////////////////////
//#define ANIMAL_LOVE
//#define KLARA
//#define ANIMALTRONIEK_KREEFT (1)
//#define ANIMALTRONIEK_VIS (1)
//#define AMI (1)
#define LUMI (1)
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef LUMI
#define BOARD_V2 (1)
//#define USE_9685 (1)
#define USE_9635 (1)
//#define USE_MOTOR (1)
//#define USE_CROSS_MIXING (1)
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
// for the audio module. Typically we use both (a sample and loop player)
#define USE_AUDIO (1)

// RF is either CRSF or (older) APC220 radio
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 3
#define SWITCH_CHANNEL 4 //second switch channel will be +1, next up + 2 and + 3

#define ANIMATION_KEY (32) // set the correct animation key here. should be in last of 4 in order to NOT be recorded

#endif



#ifdef AMI
// old 
#include "Audio.h"
#define BOARD_V2 (1)
//#define USE_MOTOR (1)
//#define USE_CROSS_MIXING (1)
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


// Actions can only be coupled with audio when the players are enabled
// note the board will only start when audio players are available
#define USE_AUDIO (1)

#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

#define ANIMATION_KEY (24) // set the correct animation key here. should be in last of 4 in order to NOT be recorded

#endif

////// hardware specifics for animaltroniek
#ifdef ANIMALTRONIEK_KREEFT
// for using DC motors, for example Electromen drives
// animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// two inputs for direction and 1 input for velocity.
// #define BOARD_V2 (1)
#define BOARD_V1 (1)
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



// Actions can only be coupled with audio when the players are enabled
// note the board will only start when audio players are available
//#define USE_AUDIO (1)

#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

#define ANIMATION_KEY (24) // set the correct animation key here. should be in last of 4 in order to NOT be recorded
#endif





#ifdef ANIMALTRONIEK_VIS
// for using DC motors, for example Electromen drives
// animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// two inputs for direction and 1 input for velocity.
// #define BOARD_V2 (1)
#define BOARD_V1 (1)
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
//#define USE_AUDIO (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

#define ANIMATION_KEY (24)


#endif


