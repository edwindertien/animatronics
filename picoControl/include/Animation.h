#ifndef __ANIMATION_H__
#define __ANIMATION_H__
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "crsf_channels.h"

//#define ANIMATION_DEBUG (1)

#define PAUSE 600 // Pause after each complete loop (in loops)

// animationStep — one 20Hz frame of recorded channel data.
// 9 bytes per step, stored in PROGMEM.
// Field names match crsf_channels.h semantics.
// Binary layout is identical to the original struct so all existing
// Track-xxx.h files work without modification.
typedef struct {
  uint8_t axis_x;       // CRSF_CH_AXIS_X1      (0-255, 127=centre)
  uint8_t axis_y;       // CRSF_CH_AXIS_Y1      (0-255, 127=centre)
  uint8_t nunchuck;     // CRSF_CH_NUNCHUCK_BTN (0=none 64=Z 128=C 192=both)
  uint8_t keypad_lo;    // CRSF_CH_KEYPAD_LO    (bitmask bits 0-7)
  uint8_t analog1;      // CRSF_CH_ANALOG1      (volume/speed pot, 0-255)
  uint8_t sw_mux_0_3;   // CRSF_CH_SW_MUX_0_3   (mux ch 0-3, 2 bits each)
  uint8_t sw_mux_4_7;   // CRSF_CH_SW_MUX_4_7   (mux ch 4-7)
  uint8_t sw_mux_8_11;  // CRSF_CH_SW_MUX_8_11  (mux ch 8-11)
  uint8_t sw_mux_12_15; // CRSF_CH_SW_MUX_12_15 (mux ch 12-15)
} animationStep;

class Animation {
  public:
    Animation(const animationStep* animationData, int totalSteps);

    void start();
    void stop();
    void update();

    bool isPlaying();
    bool isPaused();
    uint32_t elapsedSeconds() { return playing ? (millis() - startTime) / 1000 : 0; }

  private:
    const animationStep* animationData;
    int totalSteps;
    int currentStep;
    unsigned long startTime;
    unsigned long animationTimer;
    bool playing;
    bool paused;
};

#ifdef ANIMATION_KEY
extern const animationStep defaultAnimation[DEFAULT_STEPS] PROGMEM;
#endif

#ifdef EXPO_KEY
extern const animationStep expoAnimation[EXPO_STEPS] PROGMEM;
#endif

#endif