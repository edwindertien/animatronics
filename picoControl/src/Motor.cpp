#include "Motor.h"

// Constructor to initialize the motor pins
Motor::Motor(int pinA, int pinB, int pwmPin) {
  this->pinA = pinA;
  this->pinB = pinB;
  this->pwmPin = pwmPin;
  
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  
  analogWrite(pwmPin, 0); // Set initial speed to 0
  digitalWrite(pinA, LOW); // Set initial direction to LOW
  digitalWrite(pinB, LOW);
}

// Method to set motor speed (positive for forward, negative for reverse, 0 for stop)
void Motor::setSpeed(int value) {
  if (value > 0) {
    digitalWrite(pinA, HIGH);  // Set direction to forward
    digitalWrite(pinB, LOW);
    analogWrite(pwmPin, value); // Set speed
  }
  else if (value < 0) {
    digitalWrite(pinA, LOW);   // Set direction to reverse
    digitalWrite(pinB, HIGH);
    analogWrite(pwmPin, -value); // Set speed (positive value for reverse)
  }
  else {
    analogWrite(pwmPin, 0); // Stop motor
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
  }
}

int getLeftValueFromCrossMix(int speed, int direction){
    float turnScalingFactor = 1.0 / (1.0 + abs(speed) / float(maxSpeed));
  
    // Adjust turn rate by scaling it with the forward speed
    int scaledTurn = direction * turnScalingFactor;
  
    // Calculate left and right motor speeds
    int leftSpeed = speed + scaledTurn;    // Left motor speed
  
    // Constrain speeds to be between -255 and 255
    return constrain(leftSpeed, -255, 255);
    

}
int getRightValueFromCrossMix(int speed, int direction){
    float turnScalingFactor = 1.0 / (1.0 + abs(speed) / float(maxSpeed));
  
    // Adjust turn rate by scaling it with the forward speed
    int scaledTurn = direction * turnScalingFactor;
  
    // Calculate left and right motor speeds
    int rightSpeed = speed - scaledTurn;   // Right motor speed
  
  return constrain(rightSpeed, -255, 255);
}