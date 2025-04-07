#include "Animation.h"
#include "Track-kreeft.h"
// Constructor to initialize animation data and total steps
Animation::Animation(const animationStep* animationData, int totalSteps) {
  this->animationData = animationData;
  this->totalSteps = totalSteps;
  this->currentStep = 0;
  this->playing = false;
  this->animationTimer = 0;
}

// Start the animation
void Animation::start() {
  if (!playing) {
    playing = true;
    startTime = millis();
    currentStep = 0; // Start from the first step
  }
}

// Stop the animation
void Animation::stop() {
  playing = false;
  currentStep = 0; // Reset to the first step
  animationTimer = 0;
}

// Update function to be called in the main loop
void Animation::update() {
  if (playing) {
    // Read the current animation step from the program memory
    uint8_t xSetpoint = pgm_read_byte(&animationData[currentStep].xSetpoint);
    uint8_t ySetpoint = pgm_read_byte(&animationData[currentStep].ySetpoint);
    uint8_t buttons = pgm_read_byte(&animationData[currentStep].buttons);
    uint8_t keypad = pgm_read_byte(&animationData[currentStep].keypad);
    uint8_t volume = pgm_read_byte(&animationData[currentStep].volume);
    uint8_t switches1 = pgm_read_byte(&animationData[currentStep].switches1);
    uint8_t switches2 = pgm_read_byte(&animationData[currentStep].switches2);
    uint8_t switches3 = pgm_read_byte(&animationData[currentStep].switches3);
    uint8_t switches4 = pgm_read_byte(&animationData[currentStep].switches4);

    // You would update the output pins or internal state here, for example:
    // updateOutputs(xSetpoint, ySetpoint, buttons, switches);
    // Example:
    Serial.print("->");
    Serial.print(currentStep);
    Serial.print("-{");
    Serial.print(xSetpoint);
    Serial.print(','); Serial.print(ySetpoint);
    Serial.print(','); Serial.print(buttons);
    Serial.print(','); Serial.print(keypad);
    Serial.println("},");

    // Move to the next step after the timer reaches a certain threshold
    animationTimer++;
    if (currentStep < (totalSteps - 1)) {
      currentStep++;
    }

    // If we've reached the last step, and the pause time has elapsed, restart
    if (currentStep == (totalSteps - 1) && animationTimer > (totalSteps + PAUSE)) {
      currentStep = 0;
      animationTimer = 0;
      Serial.println("---------- Restarting Animation ----------");
    }
  } else {
    animationTimer = 0;
  }
}

// Check if the animation is currently playing
bool Animation::isPlaying() {
  return playing;
}

