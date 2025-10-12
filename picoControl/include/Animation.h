#ifndef __ANIMATION_H__
#define __ANIMATION_H__
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <config.h>

//#define ANIMATION_DEBUG (1)

#define PAUSE 600 // Pause after each complete loop (in loops)

typedef struct {
  uint8_t xSetpoint;
  uint8_t ySetpoint;
  uint8_t buttons;
  uint8_t keypad;
  uint8_t volume;
  uint8_t switches1;
  uint8_t switches2;
  uint8_t switches3;
  uint8_t switches4;
} animationStep;

class Animation {
  public:
    // Constructor
    Animation(const animationStep* animationData, int totalSteps);

    // Methods to control the animation
    void start();
    void stop();
    void update();

    // State tracking
    bool isPlaying();
    bool isPaused();
    
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