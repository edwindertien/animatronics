#pragma once
#define DEBUG (1)

#include "Action.h"
#include "ActionSequence.h"

// note that this definition has its consequences in the main.cpp and also in action.h (where
// each action mapping is given)
// the animation tracks are added in the file animation.cpp, the number of steps in the
// header file animation.h (so check those before compiling.)
// Two versions of the board currently: 
// V1.0 misses the wire between radio VCC and 3.3V
// V2.0 ... 
// V3.0 (not populated yet)
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DESKLIGHT
// the Waveshare motors
//#define USE_DDSM (1)
#define USE_STS (1)
#define BOARD_V2 (1)
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

#ifdef EXPERIMENT
//#define USE_DDSM (1)
//#define USE_STS (1)
//#define ROBOTIS (1)
#define CAN_DRIVER (1)
#define CUBEMARS (1)
#define BOARD_V1 (1)
#define USE_RS485 (1)
#define RS485_BAUD 57600
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

#ifdef ANIMAL_LOVE
#define BOARD_V2 (1)
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//#define USE_9685 (1)
//#define USE_9635 (1)
#define USE_MOTOR (1)
//#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 130  // was 60 used for scaling in the cross-mix function
#define HIGH_SPEED 160 // was 90  
// when no scaling used:
#define MAX_SPEED 50

// for serial output on the RJ45 socket, goes to APC220 RF interface
#define USE_RS485 (1)
#define RS485_BAUD 9600
#define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
// for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
#define USE_OLED (1)
// only use the encoder when these pins are not used for controlling separate motors
//#define USE_ENCODER (1)
// for the audio module. Typically we use both (a sample and loop player)
//#define USE_AUDIO (1)

// RF is either CRSF or (older) APC220 radio
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

#define ANIMATION_KEY (30) // set the correct animation key here. should be in last of 4 in order to NOT be recorded
#define EXPO_KEY (15) // set the pin to trigger the expo animation using a switch
#define DEFAULT_STEPS 967 
#define EXPO_STEPS 985

#define BRAKE_TIMEOUT 30 // in loops of 20Hz, so 1.5 sec
extern Motor motorLeft;
extern Motor motorRight;
extern Motor tandkrans;
void configureMotors();
#endif


#ifdef LUMI
#define BOARD_V2 (1)
//#define USE_9685 (1)
//#define USE_9635 (1)
//#define USE_MOTOR (1)
//#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60  // used for scaling in the cross-mix function
#define HIGH_SPEED 90 
// when no scaling used:
#define MAX_SPEED 50
#define USB_JOYSTICK
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 127, 127, 127, 0, 127, 127, 0, 0, 127, 0, 0, 0, 0, 0, 0};
// for serial output on the RJ45 socket, goes to CAT5 ethernet cable
//#define USE_RS485 (1)
//#define RS_485_BAUD 57600
// for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
#define USE_OLED (1)
// only use the encoder when these pins are not used for controlling separate motors
//#define USE_ENCODER (1)
// for the audio module. Typically we use both (a sample and loop player)
#define USE_AUDIO (1)

// RF is either CRSF or (older) APC220 radio
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

//#define KEYPAD_CHANNEL 3
//#define VOLUME_CHANNEL 3
#define SWITCH_CHANNEL 12 //second switch channel will be +1, next up + 2 and + 3

//#define ANIMATION_KEY (32) // set the correct animation key here. should be in last of 4 in order to NOT be recorded
//#define STEPS 1

#define NUM_TRACKS 15
const String tracklist[NUM_TRACKS] = 
{
  "/mp3/01-int.mp3",
  "/mp3/02-dro.mp3",
  "/mp3/03-maz.mp3",
  "/mp3/04-sco.mp3",
  "/mp3/05-spi.mp3",
  "/mp3/06-pat.mp3",
  "/mp3/07-moe.mp3",
  "/mp3/08-wal.mp3",
  "/mp3/09-poo.mp3",
  "/mp3/10-cer.mp3",
  "/mp3/11-opt.mp3",
  "/mp3/12-kar.mp3",
  "/mp3/13-and.mp3",
  "/mp3/14-mid.mp3",
  "/mp3/15-ora.mp3",
};
#define NUM_SAMPLES 6
const String samplelist[NUM_SAMPLES] = 
{
  "/mp3/01-alm.mp3",
  "/mp3/02-ang.mp3",
  "/mp3/03-slp.mp3",
  "/mp3/04-mov.mp3",
  "/mp3/05-noo.mp3",
  "/mp3/06-yes.mp3"
};

#endif



#ifdef AMI
// old 
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#define BOARD_V2 (1)
//#define BOARD_V1 (1)
#define EXTRA_RELAY (1) 
//#define USE_MOTOR (1)
//#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60  // used for scaling in the cross-mix function
#define HIGH_SPEED 90 
// when no scaling used:
#define MAX_SPEED 50

// for serial output on the RJ45 socket, using CAT5 cable
#define USE_RS485 (1)
#define RS485_BAUD 57600
//#define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
// for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
#define USE_OLED (1)
// only use the encoder when these pins are not used for controlling separate motors
//#define USE_ENCODER (1)

// Actions can only be coupled with audio when the players are enabled
// note the board will only start when audio players are available
#define USE_AUDIO (1)

#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

#define ANIMATION_KEY (24) // set the correct animation key here. should be in last of 4 in order to NOT be recorded
#define DEFAULT_STEPS 967 
#define EXPO_STEPS 985

// important mapping of actions, buttons, relay channels and sounds
#define NUM_ACTIONS 17

// Declarations only – definitions live in config.cpp
extern Action myActionList[NUM_ACTIONS];
extern ActionSequence looking;

// Set up any sequences (defined in config.cpp)
void configureSequences();

#endif  // AMI


////// hardware specifics for animaltroniek
#ifdef ANIMALTRONIEK_KREEFT
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// for using DC motors, for example Electromen drives
// animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// two inputs for direction and 1 input for velocity.
// #define BOARD_V2 (1)
#define BOARD_V1 (1)
//#define USE_9685
#define USE_MOTOR (1)
#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60  // used for scaling in the cross-mix function
#define HIGH_SPEED 90 
// when no scaling used:
#define MAX_SPEED 50

// for serial output on the RJ45 socket
#define USE_RS485 (1)
#define RS485_BAUD 57600
#define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
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
#define DEFAULT_STEPS 985 
extern Motor motorLeft;
extern Motor motorRight;

void configureMotors();
//extern Motor tandkrans;
#endif


#ifdef ANIMALTRONIEK_VIS
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// for using DC motors, for example Electromen drives
// animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// two inputs for direction and 1 input for velocity.
// #define BOARD_V2 (1)
#define BOARD_V1 (1)
//#define USE_9685
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
#define RS485_BAUD 57600
#define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
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
#define DEFAULT_STEPS 1005  //
extern Motor motorLeft;
extern Motor motorRight;
//extern Motor tandkrans;
#endif




// #pragma once
// #define DEBUG (1)

// #include "Action.h"
// #include "ActionSequence.h"

// // note that this definition has its consequences in the main.cpp and also in action.h (where
// // each action mapping is given)
// // the animation tracks are added in the file animation.cpp, the number of steps in the
// // header file animation.h (so check those before compiling.)
// // Two versions of the board currently: 
// // V1.0 misses the wire between radio VCC and 3.3V
// // V2.0 ... 
// // V3.0 (not populated yet)
// //////////////////////////////////////////////////////////////////////////////////////////////

// //////////////////////////////////////////////////////////////////////////////////////////////

// #ifdef DESKLIGHT
// // the Waveshare motors
// //#define USE_DDSM (1)
// #define USE_STS (1)
// #define BOARD_V2 (1)
// #define USE_OLED (1)
// #define USE_CRSF (1)
// #define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 
// #define NUM_CHANNELS 16
// const int saveValues[NUM_CHANNELS] = { 127, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// #endif

// #ifdef EXPERIMENT
// //#define USE_DDSM (1)
// //#define USE_STS (1)
// //#define ROBOTIS (1)
// #define CAN_DRIVER (1)
// #define CUBEMARS (1)
// #define BOARD_V1 (1)
// #define USE_RS485 (1)
// #define RS485_BAUD 57600
// #define USE_OLED (1)
// #define USE_CRSF (1)
// #define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 
// #define NUM_CHANNELS 16
// const int saveValues[NUM_CHANNELS] = { 127, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// #endif

// #ifdef ANIMAL_LOVE
// #define BOARD_V2 (1)
// #define NUM_CHANNELS 16
// const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// //#define USE_9685 (1)
// //#define USE_9635 (1)
// #define USE_MOTOR (1)
// //#define USE_CROSS_MIXING (1)
// #define USE_SPEEDSCALING (1)
// #define LOW_SPEED 130  // was 60 used for scaling in the cross-mix function
// #define HIGH_SPEED 160 // was 90  
// // when no scaling used:
// #define MAX_SPEED 50

// // for serial output on the RJ45 socket, goes to APC220 RF interface
// #define USE_RS485 (1)
// #define RS485_BAUD 9600
// #define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
// // for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
// #define USE_OLED (1)
// // only use the encoder when these pins are not used for controlling separate motors
// //#define USE_ENCODER (1)
// // for the audio module. Typically we use both (a sample and loop player)
// //#define USE_AUDIO (1)

// // RF is either CRSF or (older) APC220 radio
// #define USE_CRSF (1)
// #define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

// #define KEYPAD_CHANNEL 3
// #define VOLUME_CHANNEL 4
// #define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

// #define ANIMATION_KEY (30) // set the correct animation key here. should be in last of 4 in order to NOT be recorded
// #define EXPO_KEY (15) // set the pin to trigger the expo animation using a switch
// #define DEFAULT_STEPS 967 
// #define EXPO_STEPS 985
// #endif


// #ifdef LUMI
// #define BOARD_V2 (1)
// //#define USE_9685 (1)
// //#define USE_9635 (1)
// //#define USE_MOTOR (1)
// //#define USE_CROSS_MIXING (1)
// #define USE_SPEEDSCALING (1)
// #define LOW_SPEED 60  // used for scaling in the cross-mix function
// #define HIGH_SPEED 90 
// // when no scaling used:
// #define MAX_SPEED 50
// #define USB_JOYSTICK
// #define NUM_CHANNELS 16
// const int saveValues[NUM_CHANNELS] = { 127, 127, 127, 127, 0, 127, 127, 0, 0, 127, 0, 0, 0, 0, 0, 0};
// // for serial output on the RJ45 socket, goes to CAT5 ethernet cable
// //#define USE_RS485 (1)
// //#define RS_485_BAUD 57600
// // for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
// #define USE_OLED (1)
// // only use the encoder when these pins are not used for controlling separate motors
// //#define USE_ENCODER (1)
// // for the audio module. Typically we use both (a sample and loop player)
// #define USE_AUDIO (1)

// // RF is either CRSF or (older) APC220 radio
// #define USE_CRSF (1)
// #define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

// //#define KEYPAD_CHANNEL 3
// //#define VOLUME_CHANNEL 3
// #define SWITCH_CHANNEL 12 //second switch channel will be +1, next up + 2 and + 3

// //#define ANIMATION_KEY (32) // set the correct animation key here. should be in last of 4 in order to NOT be recorded
// //#define STEPS 1

// #define NUM_TRACKS 15
// const String tracklist[NUM_TRACKS] = 
// {
//   "/mp3/01-int.mp3",
//   "/mp3/02-dro.mp3",
//   "/mp3/03-maz.mp3",
//   "/mp3/04-sco.mp3",
//   "/mp3/05-spi.mp3",
//   "/mp3/06-pat.mp3",
//   "/mp3/07-moe.mp3",
//   "/mp3/08-wal.mp3",
//   "/mp3/09-poo.mp3",
//   "/mp3/10-cer.mp3",
//   "/mp3/11-opt.mp3",
//   "/mp3/12-kar.mp3",
//   "/mp3/13-and.mp3",
//   "/mp3/14-mid.mp3",
//   "/mp3/15-ora.mp3",
// };
// #define NUM_SAMPLES 6
// const String samplelist[NUM_SAMPLES] = 
// {
//   "/mp3/01-alm.mp3",
//   "/mp3/02-ang.mp3",
//   "/mp3/03-slp.mp3",
//   "/mp3/04-mov.mp3",
//   "/mp3/05-noo.mp3",
//   "/mp3/06-yes.mp3"
// };

// #endif



// #ifdef AMI
// // old 
// #include "Audio.h"
// #define NUM_CHANNELS 16
// const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// #define BOARD_V2 (1)
// //#define BOARD_V1 (1)
// #define EXTRA_RELAY (1) 
// //#define USE_MOTOR (1)
// //#define USE_CROSS_MIXING (1)
// #define USE_SPEEDSCALING (1)
// #define LOW_SPEED 60  // used for scaling in the cross-mix function
// #define HIGH_SPEED 90 
// // when no scaling used:
// #define MAX_SPEED 50

// // for serial output on the RJ45 socket, using CAT5 cable
// #define USE_RS485 (1)
// #define RS485_BAUD 57600
// //#define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
// // for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
// #define USE_OLED (1)
// // only use the encoder when these pins are not used for controlling separate motors
// //#define USE_ENCODER (1)

// // important mapping of actions, buttons, relay channels and sounds


// // Actions can only be coupled with audio when the players are enabled
// // note the board will only start when audio players are available
// #define USE_AUDIO (1)


// #define USE_CRSF (1)
// #define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

// #define KEYPAD_CHANNEL 3
// #define VOLUME_CHANNEL 4
// #define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

// #define ANIMATION_KEY (32) // set the correct animation key here. should be in last of 4 in order to NOT be recorded
// #define DEFAULT_STEPS 967 
// #define EXPO_STEPS 985

// // important mapping of actions, buttons, relay channels and sounds
// #define NUM_ACTIONS 16
// Action myActionList[NUM_ACTIONS] = {
//   Action(2, -1, DIRECT, nullptr, 100, 1, &player1), // track 1
//   Action(4, -1, DIRECT, nullptr, 100, 2, &player1), // track 2
//   Action(6, -1, DIRECT, nullptr, 100, 4, &player1), // track 3
//   Action(0, 11, DIRECT), // zwaailicht
//   Action(16, 4, DIRECT),  // achterklep open
//   Action(17, 5, DIRECT), // achterklep dicht
//   Action(12, 0, DIRECT),  // arm uit 
//   Action(13, 1, DIRECT), // arm in
//   Action(10, 22, DIRECT),  // motorkap open
//   Action(11, 23, DIRECT), // motorkap dicht
//   Action(14, 14, DIRECT), // lift up
//   Action(15,21,DIRECT), // elevator release
//   Action('0',20,DIRECT), // elevator release back
//   Action(8,15, DIRECT), // vleugeldeur
//   Action(9,12, DIRECT),
//   Action('4',19,DIRECT),
//   // Action('2', -1, TRIGGER, nullptr, 100, 1, &player2),
//   // Action('3', 2, DIRECT, nullptr, 100, 2, &player2),
//   // Action('4', 3, DIRECT, nullptr, 100, 3, &player2),
//   // Action('5', 4, DIRECT, nullptr, 100, 4, &player2),
//   // //Action('1', -1, DIRECT, &tandkrans, -100),
//   // //Action('2', -1, DIRECT, &tandkrans, -100),
//   // Action('1', 16, DIRECT)
//   //Action('4', 4, DIRECT),
//   //Action(10, 4, DIRECT), // on button s
//   //Action(11, 5, DIRECT),
// };
// ActionSequence looking('5', TOGGLE, true);   // button '5', toggle, loop

// inline void configureSequences() {
//     // sequence uses actions by pointer into myActionList
//     looking.addEvent(0,   EVENT_START, &myActionList[15]);
//     looking.addEvent(300, EVENT_STOP,  &myActionList[15]);
//     looking.addEvent(600, EVENT_STOP,  &myActionList[15]);
// }

// #endif

// ////// hardware specifics for animaltroniek
// #ifdef ANIMALTRONIEK_KREEFT
// #define NUM_CHANNELS 16
// const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// // for using DC motors, for example Electromen drives
// // animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// // two inputs for direction and 1 input for velocity.
// // #define BOARD_V2 (1)
// #define BOARD_V1 (1)
// //#define USE_9685
// #define USE_MOTOR (1)
// #define USE_CROSS_MIXING (1)
// #define USE_SPEEDSCALING (1)
// #define LOW_SPEED 60  // used for scaling in the cross-mix function
// #define HIGH_SPEED 90 
// // when no scaling used:
// #define MAX_SPEED 50

// // for serial output on the RJ45 socket
// #define USE_RS485 (1)
// #define RS485_BAUD 57600
// #define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
// // for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
// #define USE_OLED (1)
// // only use the encoder when these pins are not used for controlling separate motors
// //#define USE_ENCODER (1)

// // Actions can only be coupled with audio when the players are enabled
// // note the board will only start when audio players are available
// //#define USE_AUDIO (1)

// #define USE_CRSF (1)
// #define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

// #define KEYPAD_CHANNEL 3
// #define VOLUME_CHANNEL 4
// #define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

// #define ANIMATION_KEY (24) // set the correct animation key here. should be in last of 4 in order to NOT be recorded
// #define STEPS 985 
// #endif


// #ifdef ANIMALTRONIEK_VIS
// #define NUM_CHANNELS 16
// const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// // for using DC motors, for example Electromen drives
// // animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// // two inputs for direction and 1 input for velocity.
// // #define BOARD_V2 (1)
// #define BOARD_V1 (1)
// //#define USE_9685
// #define USE_MOTOR (1)
// #define USE_CROSS_MIXING (1)
// #define USE_SPEEDSCALING (1)
// #define USE_KEYPAD_SPEED (1) // use '#' for speed mode
// #define LOW_SPEED 60  // used for scaling in the cross-mix function
// #define HIGH_SPEED 80 
// // when no scaling used:
// #define MAX_SPEED 50

// // for serial output on the RJ45 socket
// #define USE_RS485 (1)
// #define RS485_BAUD 57600
// #define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
// // for the OLED. check the &Wire or &Wire1 (for the latest board). Also check the resolution
// #define USE_OLED (1)
// // only use the encoder when these pins are not used for controlling separate motors
// //#define USE_ENCODER (1)
// //#define USE_AUDIO (1)
// #define USE_CRSF (1)
// #define CRSF_CHANNEL_OFFSET 3 //experimental offset needed to remap correctly... 

// #define KEYPAD_CHANNEL 3
// #define VOLUME_CHANNEL 4
// #define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3

// #define ANIMATION_KEY (24)
// #define STEPS 1005  //
// #endif
