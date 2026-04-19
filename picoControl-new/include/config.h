#pragma once
// config.h — hardware capability flags per vehicle.
// Only #defines, saveValues, and feature flags live here.
// No Action instances, no Motor instances, no extern declarations for
// vehicle-specific objects — those all live in platform_xxx.cpp / platform.h.

// USE_AUDIO levels:  0 = no audio
//                    1 = player1 only
//                    2 = player1 + player2
#if defined(AMI) || defined(LUMI)
  #define USE_AUDIO 2
#elif defined(SCUBA)
  #define USE_AUDIO 1
#else
  #define USE_AUDIO 0
#endif

// Background tracks — must be defined before Action.h is included
// so Audio.h sees them when it is first parsed.
#ifdef SCUBA
  #define BACKGROUND_TRACK_1 1
#endif

// Now pull in Action/ActionSequence — USE_AUDIO is defined above so Audio.h
// will see the correct level when Action.h includes it.
#include "Action.h"
#include "ActionSequence.h"

//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DESKLIGHT
#define USE_STS (1)
#define BOARD_V3 (1)
#define USE_OLED (1)
#define OLED_ROTATE (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define NUM_CHANNELS 16
extern const int saveValues[];
#endif

#ifdef STOFZUIGER
#define BOARD_V3 (1)
#define USE_MOTOR (1)
//#define USE_M5_SERVOS (1)
//#define USE_SPEEDSCALING (1)
#define LOW_SPEED 255
#define HIGH_SPEED 255
#define MAX_SPEED 255
#define BRAKE_TIMEOUT 30
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define NUM_CHANNELS 16
extern const int saveValues[];
extern Motor motorLeft;
extern Motor motorRight;
void configureMotors();
#endif

#ifdef WASHMACHINE
#define BOARD_V3 (1)
#define USE_MOTOR (1)
#define USE_M5_SERVOS (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 255
#define HIGH_SPEED 255
#define MAX_SPEED 255
#define BRAKE_TIMEOUT 30
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define NUM_CHANNELS 16
extern const int saveValues[];
extern Motor motorLeft;
extern Motor motorRight;
extern Motor trommel;
void configureMotors();
#endif

#ifdef ANIMAL_LOVE
#define BOARD_V2 (1)
#define NUM_CHANNELS 16
extern const int saveValues[];
#define USE_MOTOR (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 130
#define HIGH_SPEED 160
#define MAX_SPEED 50
#define USE_RS485 (1)
#define RS485_BAUD 9600
#define BUFFER_PASSTHROUGH 9
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5
#define ANIMATION_KEY (30)
#define EXPO_KEY (15)
#define DEFAULT_STEPS 967
#define EXPO_STEPS 985
#define ANIMATION_TRACK_H "Track-animalove.h"
#define BRAKE_TIMEOUT 30
#define NUM_ACTIONS 6
extern Motor motorLeft;
extern Motor motorRight;
extern Motor tandkrans;
void configureMotors();
extern Action myActionList[NUM_ACTIONS];
#endif

#ifdef LUMI
#define BOARD_V2 (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60
#define HIGH_SPEED 90
#define MAX_SPEED 50
#define USB_JOYSTICK
#define NUM_CHANNELS 16
extern const int saveValues[];
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define SWITCH_CHANNEL 12
#define VOLUME_CHANNEL 4
#define NUM_TRACKS 15
extern const String tracklist[NUM_TRACKS];
#define NUM_SAMPLES 6
extern const String samplelist[NUM_SAMPLES];
#define NUM_ACTIONS 6
extern Action myActionList[NUM_ACTIONS];
#endif

#ifdef SCUBA
#define NUM_CHANNELS 16
extern const int saveValues[];
#define BOARD_V2 (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60
#define HIGH_SPEED 90
#define MAX_SPEED 50
#define USE_RS485 (1)
#define RS485_BAUD 57600
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5
#define NUM_ACTIONS 13
extern Action myActionList[NUM_ACTIONS];
extern ActionSequence jaws;
#endif

#ifdef AMI
#define NUM_CHANNELS 16
extern const int saveValues[];
#define BOARD_V2 (1)
#define EXTRA_RELAY (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60
#define HIGH_SPEED 90
#define MAX_SPEED 50
#define USE_RS485 (1)
#define RS485_BAUD 57600
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5
#define NUM_ACTIONS 22
extern Action myActionList[NUM_ACTIONS];
extern ActionSequence looking;
#endif

#ifdef ANIMALTRONIEK_KREEFT
#define NUM_CHANNELS 16
extern const int saveValues[];
#define BOARD_V1 (1)
#define USE_MOTOR (1)
#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60
#define HIGH_SPEED 90
#define MAX_SPEED 50
#define USE_RS485 (1)
#define RS485_BAUD 57600
#define BUFFER_PASSTHROUGH 9
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5
#define ANIMATION_KEY (24)
#define DEFAULT_STEPS 985
#define ANIMATION_TRACK_H "Track-kreeft.h"
#define BRAKE_TIMEOUT 30
#define NUM_ACTIONS 2
extern Motor motorLeft;
extern Motor motorRight;
void configureMotors();
extern Action myActionList[NUM_ACTIONS];
#endif

#ifdef ANIMALTRONIEK_VIS
#define NUM_CHANNELS 16
extern const int saveValues[];
#define BOARD_V1 (1)
#define USE_MOTOR (1)
#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60
#define HIGH_SPEED 80
#define MAX_SPEED 50
#define USE_RS485 (1)
#define RS485_BAUD 57600
#define BUFFER_PASSTHROUGH 9
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5
#define BRAKE_TIMEOUT 20
#define NUM_ACTIONS 2
#define ANIMATION_KEY (24)
#define DEFAULT_STEPS 1005
#define ANIMATION_TRACK_H "Track-vis.h"
extern Motor motorLeft;
extern Motor motorRight;
void configureMotors();
extern Action myActionList[NUM_ACTIONS];
#endif

#ifdef ANIMALTRONIEK_SCHILDPAD
#define NUM_CHANNELS 16
extern const int saveValues[];
#define BOARD_V1 (1)
#define USE_MOTOR (1)
#define USE_CROSS_MIXING (1)
#define USE_SPEEDSCALING (1)
#define LOW_SPEED 60
#define HIGH_SPEED 80
#define MAX_SPEED 50
#define USE_RS485 (1)
#define RS485_BAUD 57600
#define BUFFER_PASSTHROUGH 9
#define USE_OLED (1)
#define USE_CRSF (1)
#define CRSF_CHANNEL_OFFSET 3
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5
#define BRAKE_TIMEOUT 20
#define NUM_ACTIONS 2
#define ANIMATION_KEY (24)
#define DEFAULT_STEPS 2
#define ANIMATION_TRACK_H "Track-schildpad.h"
extern Motor motorLeft;
extern Motor motorRight;
void configureMotors();
extern Action myActionList[NUM_ACTIONS];
#endif

/*
     Sample 1: fluitje         Sample 7: arm uit
     Sample 2: fietfiew         Sample 8: arm in
     Sample 3: kloinkskinkeldekikel  Sample 9: motorkap
     Sample 4: ratelratel       Sample 10: achterklep omhoog
     Sample 5:                  Sample 11: snurk
     Sample 6:                  Sample 12: deur
                                Sample 13: fly up
                                Sample 14: fly down
                                Sample 15: grill
                                Sample 16: knipoog
                                Sample 17: rook
                                Sample 18: zwaailamp
*/