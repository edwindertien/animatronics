#ifndef __ANIMATION_H__
#define __ANIMATION_H__
#include <Arduino.h>
#include <avr/pgmspace.h>

#define STEPS 164
#define PAUSE 1000 // Pause after each complete loop (milliseconds)

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
    
  private:
    const animationStep* animationData;
    int totalSteps;
    int currentStep;
    unsigned long startTime;
    unsigned long animationTimer;
    bool playing;
};

extern const animationStep defaultAnimation[STEPS] PROGMEM;

#endif