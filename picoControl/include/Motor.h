#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

const int maxSpeed = 255;
int getLeftValueFromCrossMix(int speed, int direction);
int getRightValueFromCrossMix(int speed, int direction);

#define MOTOR_DEADZONE 20

extern void writeRelay(int relay, bool state);  // has to be provided elsewhere!

class Motor {
  private:
    int pinA;   // Motor control pin A (usually for direction)
    int pinB;   // Motor control pin B (usually for direction)
    int pwmPin; // Motor PWM pin (for speed control)
    int brakeRelay;

  public:
    // Constructor to initialize the motor pins
    Motor(int pinA, int pinB, int pwmPin, int brakeRelay = -1);

    void init();
    // Method to set motor speed (positive for forward, negative for reverse, 0 for stop)
    void setSpeed(int value, bool braking);
};

#endif