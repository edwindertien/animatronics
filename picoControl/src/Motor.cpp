#include "Motor.h"
#include "PicoRelay.h"

extern PicoRelay relay;

// Constructor to initialize the motor pins
Motor::Motor(int pinA, int pinB, int pwmPin, int brakeRelay) {
  this->pinA = pinA;
  this->pinB = pinB;
  this->pwmPin = pwmPin;
  this->brakeRelay = brakeRelay;  
}

void Motor::init() {
  if(pinA >= 0)pinMode(pinA, OUTPUT);
  if(pinB >= 0)pinMode(pinB, OUTPUT);
  if(pwmPin >= 0)pinMode(pwmPin, OUTPUT);
  
  if(pwmPin >= 0)analogWrite(pwmPin, 0); // Set initial speed to 0
  if(pinA >= 0)digitalWrite(pinA, LOW); // Set initial direction to LOW
  if(pinB >= 0)digitalWrite(pinB, LOW);
  if(brakeRelay >= 0)relay.writeRelay(brakeRelay,LOW); // brake on
}

// Method to set motor speed (positive for forward, negative for reverse, 0 for stop)
void Motor::setSpeed(int value, bool braking) {
  if (value >  MOTOR_DEADZONE) {
    if(pinA >= 0)digitalWrite(pinA, HIGH);  // Set direction to forward
    if(pinB >= 0)digitalWrite(pinB, LOW);
    if(pwmPin >= 0)analogWrite(pwmPin, value); // Set speed
    if(brakeRelay >= 0)relay.writeRelay(brakeRelay,1); 
  }
  else if (value <  -MOTOR_DEADZONE) {
    if(pinA >= 0)digitalWrite(pinA, LOW);   // Set direction to reverse
    if(pinB >= 0)digitalWrite(pinB, HIGH);
    if(pwmPin >= 0)analogWrite(pwmPin, abs(value)); // Set speed (positive value for reverse)
    if(brakeRelay >= 0)relay.writeRelay(brakeRelay,1); 
  }
  else {
    if(pwmPin >= 0)analogWrite(pwmPin, 0); // Stop motor
    if(pinA >= 0)digitalWrite(pinA, LOW);
    if(pinB >= 0)digitalWrite(pinB, LOW);
    if(braking){if(brakeRelay >= 0)relay.writeRelay(brakeRelay,0); }
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