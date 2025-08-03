#pragma once
#define DEBUG (1)
// note that this definition has its consequences in the main.cpp and also in action.h (where
// each action mapping is given)
// the animation tracks are added in the file animation.cpp, the number of steps in the
// header file animation.h (so check those before compiling.)
//////////////////////////////////////////////////////////////////////////////////////////////
//#define ANIMAL_LOVE
//#define KLARA
//#define ANIMALTRONIEK_KREEFT (1)
//#define ANIMALTRONIEK_VIS (1)
//#define ANIMALTRONIEK_SCHILDPAD (1)
//#define SCUBA (1)
//#define AMI (1)
//#define LUMI (1)
//#define DESKLIGHT (1)
#define EXPERIMENT (1)
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
#define ROBOTIS (1)
#define BOARD_V2 (1)
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
#define RS_485_BAUD 9600
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
#define STEPS 967 
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
#include "Audio.h"
#define NUM_CHANNELS 16
const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#define BOARD_V2 (1)
//#define USE_MOTOR (1)
//#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60  // used for scaling in the cross-mix function
#define HIGH_SPEED 90 
// when no scaling used:
#define MAX_SPEED 50

// for serial output on the RJ45 socket, using CAT5 cable
#define USE_RS485 (1)
#define RS_485_BAUD 57600
#define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
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
#define RS_485_BAUD 57600
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
#define STEPS 985 
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
#define RS_485_BAUD 57600
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
#define STEPS 1005  //
#endif
