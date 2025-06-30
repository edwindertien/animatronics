#pragma once



#include <Arduino.h>
#include "config.h"
#ifdef USE_9685
#include <Adafruit_PWMServoDriver.h>
#else
#include <PCA9635.h>
#endif

class PicoRelay {
public:
#ifdef USE_9685
  Adafruit_PWMServoDriver pwm;
#else
  PCA9635 pwm;
#endif

  PicoRelay();

  void begin();
  void writeRelay(int relay, bool state);

#ifdef LUMI
  void joystickToRelays(int x, int y);
#endif

private:
#ifdef LUMI
  bool joystickActive = false;
  const int driveRelays[12] = {
    0b00001000,
    0b00011000,
    0b00010000,
    0b00110000,
    0b00100000,
    0b00100001,
    0b00000001,
    0b00000011,
    0b00000010,
    0b00000110,
    0b00000100,
    0b00001100,
  };
#endif
};
