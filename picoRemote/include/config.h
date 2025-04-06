/////////////////////////////////////
// Universal Remote 
// using CRSF and/or APC220 RF transmitter
// select creature here, and give below the
// matching hardware use. Currently two versions of the remote board 
// are in circulation. ALAN and KLARA have been using the
// V1.0 board with as main difference an OLED on WIRE0 instead 
// of WIRE 1 (no define yet)
#define ANIMAL_LOVE (1)
//#define KLARA (1)
//#define ALAN (1)
//#define LUMI (1)
//#define ANIMALTRONIEK (1)

#ifdef ANIMAL_LOVE
//#define DEBUG (1)
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
unsigned char channelMap[RF_MAX_CHANNEL] = {16,17,18,20, 8,19,21,23,23,23,23,23,23,23,23,23};
// originally, the latest 'universal remote' for EXOOT uses
//                          X    Y    bt  sw  vol k2a k2b k1a k1b
// int transmitBuffer[9] = {127, 127, 0,  0,  0,  0,  0,  0,  0};

int usedChannel[]   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  // used channels on the mux
int switchChannel[] = {1,1,1,1,2,2,2,2,0,0,0,0,0,0,0,0};  // switch type channels
int invertChannel[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  // values that need to be inverted
#endif

#ifdef ALAN
unsigned char channelMap[8] = {0,1,19,20,21,22,23,24}; // for ALAN
#endif

#ifdef LUMI
unsigned char channelMap[CRSF_MAX_CHANNEL] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

// if(channels[10]<64) channels[20] +=1;
// if(channels[11]<64) channels[20] +=2;
// if(channels[12]<64) channels[20] +=4;
// if(channels[15]<64) channels[20] +=8;
// if(channels[13]<64) channels[20] += 16;
// if(channels[13]>180) channels[20] +=32;
// if(channels[14]<64) channels[20] += 64;
// if(channels[14]>180) channels[20] += 128;
#endif