/////////////////////////////////////
// Universal Remote 
// using CRSF and/or APC220 RF transmitter
// select creature here, and give below the
// matching hardware use. Currently two versions of the remote board 
// are in circulation. ALAN and KLARA have been using the
// V1.0 board with as main difference an OLED on WIRE0 instead 
// of WIRE 1 (no define yet)
//
//
// V2.0 configuration details
// flash ELRS BetaFPV 2.4GHz 1W transmitter
// (for now) version 3.4.3
// set rate at 333Hz, and switches to 16/2
// telemetry (not used yet) std (1:128)
// transmission power (arbitrary for now)
// on PCB V2.0 make sure the solder jumper (bypass transmission buffer) is placed
//
// std Grove I2C has SCL and SDA swapped with respect to OLED connector (!)
// 
#define DEBUG (1)
//#define ANIMAL_LOVE (1)
//#define KLARA (1)
//#define ALAN (1)
//#define LUMI (1)
#define ANIMALTRONIEK_VIS (1)
//#define ANIMALTRONIEK_KREEFT (1)
//#define AMI (1)

#ifdef AMI
#define USE_CRSF (1)
#define DISPLAY_HEIGHT 32
//#define USE_APC (1)
//#define USE_DMX (1)
//#define USE_USB_MIDI (1)
#define STUFF_SWITCHES (1)// edit this function by hand for analog channel to switch mapping
// these will be 'stuffed' in channels 20 tm 23 (4 bytes)
#define USE_KEYPAD (1)    // 12 buttons as character in channels[20]
#define USE_OLED (1)      // check OLED size and Wire port
#define USE_NUNCHUCK (1)  // registered in channels[16],[17] and [18]
#define SLOW_MODE 1.0     // scaling factor for nunchuck. set to 1 to have a 1:1 mapping
#define FAST_MODE 1.0
#define RF_MAX_CHANNEL 16
//                                           0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
unsigned char channelMap[RF_MAX_CHANNEL] = {16,17,18,19,15,20,21,22,23,23,23,23,23,23,23,23};
// originally, the latest 'universal remote' for EXOOT uses
//                          X    Y    bt  sw  vol k2a k2b k1a k1b
// int transmitBuffer[9] = {127, 127, 0,  0,  0,  0,  0,  0,  0};

int usedChannel[]   = {1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,1};  // used channels on the mux
int switchChannel[] = {1,1,1,1,2,2,2,2,2,0,0,0,1,0,0,0};  // switch type channels
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // values that need to be inverted
#endif

#ifdef LUMI
#define USE_CRSF (1)
#define DISPLAY_HEIGHT 64
#define USE_MAX17048 (1)
#define USE_ENCODER (1)
#define ENCODER_CHANNEL 24
//#define USE_APC (1)
//#define USE_DMX (1)
//#define USE_USB_MIDI (1)
#define STUFF_SWITCHES (1)// edit this function by hand for analog channel to switch mapping
// these will be 'stuffed' in channels 20 tm 23 (4 bytes)
//#define USE_KEYPAD (1)    // 12 buttons as character in channels[20]
#define USE_OLED (1)      // check OLED size and Wire port
//#define USE_NUNCHUCK (1)  // registered in channels[16],[17] and [18]
#define SLOW_MODE 1.0     // scaling factor for nunchuck. set to 1 to have a 1:1 mapping
#define FAST_MODE 1.0
#define RF_MAX_CHANNEL 16 // note that the last two channels are overwitten by RSSI and battery
//                                           0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
unsigned char channelMap[RF_MAX_CHANNEL] = { 0, 1, 2, 3, 6, 5, 4, 7, 8, 9,21,22,23,24,23,23};
// originally, the latest 'universal remote' for EXOOT uses
//                          X    Y    bt  sw  vol k2a k2b k1a k1b
// int transmitBuffer[9] = {127, 127, 0,  0,  0,  0,  0,  0,  0};

int usedChannel[]   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  // used channels on the mux
int switchChannel[] = {0,0,0,0,1,0,0,0,0,0,1,1,1,2,2,1};  // switch type channels
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // values that need to be inverted
#endif




#ifdef ANIMALTRONIEK_KREEFT
#define DISPLAY_HEIGHT 32
#define USE_CRSF (1)
#define USE_MAX17048 (1)
//#define USE_APC (1)
//#define USE_DMX (1)
//#define USE_USB_MIDI (1)
#define STUFF_SWITCHES (1)// edit this function by hand for analog channel to switch mapping
// these will be 'stuffed' in channels 20 tm 23 (4 bytes)
#define USE_KEYPAD (1)    // 12 buttons as character in channels[20]
#define USE_OLED (1)      // check OLED size and Wire port
#define USE_NUNCHUCK (1)  // registered in channels[16],[17] and [18]
#define SLOW_MODE 1.0     // scaling factor for nunchuck. set to 1 to have a 1:1 mapping
#define FAST_MODE 1.0
#define RF_MAX_CHANNEL 16
//                                           0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
unsigned char channelMap[RF_MAX_CHANNEL] = {16,17,18,19,19,20,21,22,23,23,23,23,23,23,23,23};
// originally, the latest 'universal remote' for EXOOT uses
//                          X    Y    bt  sw  vol k2a k2b k1a k1b
// int transmitBuffer[9] = {127, 127, 0,  0,  0,  0,  0,  0,  0};

int usedChannel[]   = {1,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0};  // used channels on the mux
int switchChannel[] = {1,1,1,0,2,2,2,2,0,0,0,0,1,0,0,0};  // switch type channels
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // values that need to be inverted
#endif


#ifdef ANIMALTRONIEK_VIS
#define DISPLAY_HEIGHT 32
#define USE_CRSF (1)
#define USE_MAX17048 (1)
//#define USE_APC (1)
//#define USE_DMX (1)
//#define USE_USB_MIDI (1)
#define STUFF_SWITCHES (1)// edit this function by hand for analog channel to switch mapping
// these will be 'stuffed' in channels 20 tm 23 (4 bytes)
#define USE_KEYPAD (1)    // 12 buttons as character in channels[20]
#define USE_OLED (1)      // check OLED size and Wire port
#define USE_NUNCHUCK (1)  // registered in channels[16],[17] and [18]
#define SLOW_MODE 1.0     // scaling factor for nunchuck. set to 1 to have a 1:1 mapping
#define FAST_MODE 1.0
#define RF_MAX_CHANNEL 16
//                                           0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
unsigned char channelMap[RF_MAX_CHANNEL] = {16,17,18,19,19,20,21,22,23,23,23,23,23,23,23,23};
// originally, the latest 'universal remote' for EXOOT uses
//                          X    Y    bt  sw  vol k2a k2b k1a k1b
// int transmitBuffer[9] = {127, 127, 0,  0,  0,  0,  0,  0,  0};

int usedChannel[]   = {1,1,1,0,1,1,0,0,0,0,0,0,1,0,0,0};  // used channels on the mux
int switchChannel[] = {2,2,2,0,1,1,0,0,0,0,0,0,1,0,0,0};  // switch type channels
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // values that need to be inverted
#endif

#ifdef ANIMAL_LOVE
#define DISPLAY_HEIGHT 32
//#define DEBUG (1)
#define USE_MAX17048 (1)
#define USE_CRSF (1)
//#define USE_APC (1)
//#define USE_DMX (1)
//#define USE_USB_MIDI (1)
#define STUFF_SWITCHES (1)// edit this function by hand for analog channel to switch mapping
#define USE_KEYPAD (1)    // 12 buttons as character in channels[20]
#define USE_OLED (1)      // check OLED size and Wire port
#define USE_NUNCHUCK (1)  // registered in channels[16],[17] and [18]
#define SLOW_MODE 2.0     // scaling factor for nunchuck. set to 1 to have a 1:1 mapping
#define FAST_MODE 1.5
#define RF_MAX_CHANNEL 16
//                                           0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
unsigned char channelMap[RF_MAX_CHANNEL] = {16,17,18,19,19,20,21,22,23,23,23,23,23,23,23,23};
// originally, the latest 'universal remote' for EXOOT uses
//                          X    Y    bt  sw  vol k2a k2b k1a k1b
// int transmitBuffer[9] = {127, 127, 0,  0,  0,  0,  0,  0,  0};

int usedChannel[]   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  // used channels on the mux
int switchChannel[] = {2,2,2,2,1,1,1,2,1,1,0,0,0,0,0,1};  // switch type channels
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // values that need to be inverted
#endif

#ifdef ALAN
unsigned char channelMap[8] = {0,1,19,20,21,22,23,24}; // for ALAN
#endif

