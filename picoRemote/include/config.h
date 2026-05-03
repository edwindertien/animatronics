/////////////////////////////////////
// Universal Remote 
// using CRSF and/or APC220 RF transmitter
// select creature here, and give below the matching hardware config.
//
// V2.0 board details:
//   ELRS BetaFPV 2.4GHz 1W, ELRS 4.0.0
//   333Hz packet rate, 16ch/2 switch mode, telemetry std 1:128
//   Solder jumper (bypass TX buffer) must be closed on PCB V2.0
//   std Grove I2C has SCL and SDA swapped vs OLED connector (!)
//
// Channel protocol: see include/crsf_channels.h
//   Include that file on receiver side too — it defines all channel indices,
//   switch decoding macros, and keypad bitmask helpers.
//
#define DEBUG (1)
//#define ANIMAL_LOVE (1)
//#define KLARA (1)
//#define ALAN (1)
//#define LUMI (1)
//#define ANIMALTRONIEK_VIS (1)
//#define ANIMALTRONIEK_KREEFT (1)
#define AMI (1)
//#define SCUBA (1)
//#define EXPERIMENTAL (1)

// ── Axis sources — used in main.cpp to fill CRSF_CH_AXIS_X1/Y1/X2/Y2 ────────
// Define which internal channels[] slot feeds each CRSF axis channel.
// channels[0..15] = mux inputs, [16]=nunchuck X, [17]=nunchuck Y, [18]=nunchuck btn
// Use 255 as "not connected" → sends 127 (mid) on that CRSF channel.
#define AXIS_NOT_CONNECTED 255


#ifdef EXPERIMENTAL
#define USE_CRSF (1)
#define DISPLAY_HEIGHT 32
#define DISPLAY_ROTATE (1)
#define USE_MAX17048 (1)
#define STUFF_SWITCHES (1)
#define USE_KEYPAD (1)
#define USE_OLED (1)
#define USE_NUNCHUCK (1)
#define SLOW_MODE 1.0
#define FAST_MODE 1.0
// Axis sources (internal channels[] indices)
#define AXIS_X1_SRC  16   // nunchuck X
#define AXIS_Y1_SRC  17   // nunchuck Y
#define AXIS_X2_SRC  AXIS_NOT_CONNECTED
#define AXIS_Y2_SRC  AXIS_NOT_CONNECTED
// Analog aux sources (mux channel indices, or AXIS_NOT_CONNECTED)
#define ANALOG1_SRC  8    // vol pot
#define ANALOG2_SRC  AXIS_NOT_CONNECTED
#define ANALOG3_SRC  AXIS_NOT_CONNECTED
// Arm switch: mux channel read for CH5. Above 128 = armed.
#define ARM_MUX_CHANNEL 8

int usedChannel[]   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int switchChannel[] = {2,2,2,2,2,2,2,2,0,1,1,1,1,1,1,1};
int invertChannel[] = {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1};
#endif


#ifdef AMI
#define USE_CRSF (1)
#define DISPLAY_HEIGHT 32
#define USE_MAX17048 (1)
#define STUFF_SWITCHES (1)
#define USE_KEYPAD (1)
#define USE_OLED (1)
#define USE_NUNCHUCK (1)
#define SLOW_MODE 1.0
#define FAST_MODE 1.0
#define AXIS_X1_SRC  16
#define AXIS_Y1_SRC  17
#define AXIS_X2_SRC  AXIS_NOT_CONNECTED
#define AXIS_Y2_SRC  AXIS_NOT_CONNECTED
#define ANALOG1_SRC  15   // vol pot on mux 15
#define ANALOG2_SRC  AXIS_NOT_CONNECTED
#define ANALOG3_SRC  AXIS_NOT_CONNECTED
#define ARM_MUX_CHANNEL 15

int usedChannel[]   = {1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,1};
int switchChannel[] = {1,1,1,1,2,2,2,2,2,0,0,0,1,0,0,0};
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif


#ifdef SCUBA
#define USE_CRSF (1)
#define DISPLAY_HEIGHT 32
#define USE_MAX17048 (1)
#define STUFF_SWITCHES (1)
#define USE_KEYPAD (1)
#define USE_OLED (1)
#define USE_NUNCHUCK (1)
#define SLOW_MODE 1.0
#define FAST_MODE 1.0
#define AXIS_X1_SRC  16
#define AXIS_Y1_SRC  17
#define AXIS_X2_SRC  AXIS_NOT_CONNECTED
#define AXIS_Y2_SRC  AXIS_NOT_CONNECTED
#define ANALOG1_SRC  8    // vol pot on mux ch 8 (9th channel, counting from 1)
#define ANALOG2_SRC  AXIS_NOT_CONNECTED
#define ANALOG3_SRC  AXIS_NOT_CONNECTED
#define ARM_MUX_CHANNEL 12  // vol pot also acts as arm signal (above mid = armed)


// int usedChannel[]   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  // ch8 enabled for vol
// int switchChannel[] = {2,2,2,2,2,2,2,2,0,1,1,1,1,1,1,1};  // ch8 is analog, not a switch
// int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int usedChannel[]   = {1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,1};  // ch8 enabled for vol
int switchChannel[] = {1,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0};  // ch8 is analog, not a switch
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif


#ifdef LUMI
#define USE_CRSF (1)
#define DISPLAY_HEIGHT 64
#define USE_MAX17048 (1)
#define USE_ENCODER (1)
#define ENCODER_CHANNEL 24
#define STUFF_SWITCHES (1)
#define USE_OLED (1)
#define SLOW_MODE 1.0
#define FAST_MODE 1.0
// LUMI: two 3-axis joysticks on mux channels 0–5
#define AXIS_X1_SRC  0    // joystick 1 X
#define AXIS_Y1_SRC  1    // joystick 1 Y
#define AXIS_X2_SRC  3    // joystick 2 X  (was mux 3 in old map)
#define AXIS_Y2_SRC  2    // joystick 2 Y  (was mux 2)
#define ANALOG1_SRC  5    // joystick 1 Z or spare
#define ANALOG2_SRC  4    // joystick 2 Z or spare
#define ANALOG3_SRC  AXIS_NOT_CONNECTED
#define ARM_MUX_CHANNEL 6

int usedChannel[]   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int switchChannel[] = {0,0,0,0,1,0,0,0,0,0,1,1,1,2,2,1};
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif


#ifdef ANIMALTRONIEK_KREEFT
#define DISPLAY_HEIGHT 32
#define USE_CRSF (1)
#define USE_MAX17048 (1)
#define STUFF_SWITCHES (1)
#define USE_KEYPAD (1)
#define USE_OLED (1)
#define USE_NUNCHUCK (1)
#define SLOW_MODE 1.0
#define FAST_MODE 1.0
#define AXIS_X1_SRC  16
#define AXIS_Y1_SRC  17
#define AXIS_X2_SRC  AXIS_NOT_CONNECTED
#define AXIS_Y2_SRC  AXIS_NOT_CONNECTED
#define ANALOG1_SRC  19   // joystick sw (was vol in old map, mux 19)
#define ANALOG2_SRC  AXIS_NOT_CONNECTED
#define ANALOG3_SRC  AXIS_NOT_CONNECTED
#define ARM_MUX_CHANNEL 19

int usedChannel[]   = {1,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0};
int switchChannel[] = {1,1,1,0,2,2,2,2,0,0,0,0,1,0,0,0};
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif


#ifdef ANIMALTRONIEK_VIS
#define DISPLAY_HEIGHT 32
#define DISPLAY_ROTATE (1)
#define USE_CRSF (1)
#define USE_MAX17048 (1)
#define STUFF_SWITCHES (1)
#define USE_KEYPAD (1)
#define USE_OLED (1)
#define USE_NUNCHUCK (1)
#define SLOW_MODE 1.0
#define FAST_MODE 1.0
#define AXIS_X1_SRC  16
#define AXIS_Y1_SRC  17
#define AXIS_X2_SRC  AXIS_NOT_CONNECTED
#define AXIS_Y2_SRC  AXIS_NOT_CONNECTED
#define ANALOG1_SRC  19
#define ANALOG2_SRC  AXIS_NOT_CONNECTED
#define ANALOG3_SRC  AXIS_NOT_CONNECTED
#define ARM_MUX_CHANNEL 19

int usedChannel[]   = {1,1,1,0,1,1,0,0,0,0,0,0,1,0,0,0};
int switchChannel[] = {2,2,2,0,1,1,0,0,0,0,0,0,1,0,0,0};
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif


#ifdef ANIMAL_LOVE
#define DISPLAY_HEIGHT 32
//#define DEBUG (1)
#define USE_MAX17048 (1)
#define USE_CRSF (1)
#define STUFF_SWITCHES (1)
#define USE_KEYPAD (1)
#define USE_OLED (1)
#define USE_NUNCHUCK (1)
#define SLOW_MODE 2.0
#define FAST_MODE 1.5
#define AXIS_X1_SRC  16
#define AXIS_Y1_SRC  17
#define AXIS_X2_SRC  AXIS_NOT_CONNECTED
#define AXIS_Y2_SRC  AXIS_NOT_CONNECTED
#define ANALOG1_SRC  AXIS_NOT_CONNECTED
#define ANALOG2_SRC  AXIS_NOT_CONNECTED
#define ANALOG3_SRC  AXIS_NOT_CONNECTED
#define ARM_MUX_CHANNEL 19

int usedChannel[]   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int switchChannel[] = {2,2,2,2,1,1,1,2,1,1,1,1,1,1,1,1};
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif


#ifdef ALAN
// ALAN uses old APC220 protocol, no CRSF channel map needed
#endif