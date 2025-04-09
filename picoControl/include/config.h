//#define ANIMAL_LOVE
//#define KLARA
#define ANIMALTRONIEK (1)
#define DEBUG (1)

////// hardware specifics for animaltroniek
#ifdef ANIMALTRONIEK
// for using DC motors, for example Electromen drives
// animaltroniek uses 2 x  x EM-115-48 H-brug which uses
// two inputs for direction and 1 input for velocity.
#define USE_MOTOR (1)
#define USE_CROSS_MIXING (1)
#define MAX_SPEED 100  // used for scaling in the cross-mix function
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
#define NUM_CHANNELS 16
// at present 14 of the 16 channels are used. Enter the save values (FAILSAFE) in these arrays
//                                           0    1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
unsigned char channels[NUM_CHANNELS] =   { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//                                           X    Y nb kp vo sw sw sw sw
#define KEYPAD_CHANNEL 3
#define VOLUME_CHANNEL 4
#define SWITCH_CHANNEL 5 //second switch channel will be +1, next up + 2 and + 3
#endif