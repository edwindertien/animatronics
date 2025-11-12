#ifndef PICO_RELAY_H
#define PICO_RELAY_H

#include <Arduino.h>
#include <config.h>

#ifdef BOARD_V2
#define  USE_9635 (1)   // the 8 bit switcher
#elif defined(BOARD_V1)
#define USE_9685 (1)    // the 11 bit LED dimmer which can do servo PWM
#endif

#if defined(USE_9685)
#include <Adafruit_PWMServoDriver.h>
#elif defined(USE_9635)
#include <PCA9635.h>
#endif

class PicoRelay {
public:
    PicoRelay();
    void begin();
    void writeRelay(int relaynr, bool state);

#if defined(EXTRA_RELAY)
int relaypin[8] = {18,19,20,21,22,26,27,28};
#endif

#if defined(LUMI)
    void joystickToRelays(int x, int y);

private:
    static const uint8_t driveRelays[12];
    bool joystickActive;
#endif

private:
#if defined(USE_9685)
    Adafruit_PWMServoDriver pwm;
#elif defined(USE_9635)
    PCA9635 pwm;
#endif



};

#endif // PICO_RELAY_H