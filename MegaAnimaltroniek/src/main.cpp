/////////////////////////////////////
// Robot Receiver for animatronic
// using animatronics shield v3.1
// on an Arduino MEGA - 'toeters en bellen'
// receives commands through RS485 or RF, make sure the 
// currently in use for Animaltroniek and Animal_Love
// check config.h for specific settings. 
// 
// currently this source expects 9 channels over RF or RS485 (check baudrate)
// controlling up to 16 relays and 6 eye-servos
// servos are xL, xR, yL, yR, bL, bR
//
#include <Arduino.h>
#define DEBUG (1)
#include "Action.h"

#include "config.h"

#include <Servo.h>
Servo eyeServo[6];

#ifdef ANIMAL_LOVE
#define PORT Serial1
// uses Serial1 for radio instead of the RJ45 socket
#else
#define PORT Serial2
#endif

#include "DynamixelReader.h"
#define REMOTE_BUFFER 64
unsigned char Data[REMOTE_BUFFER];
unsigned long remoteTimeout;
int validMessage;
  
Message message = {
  .xSetpoint = 127,
  .ySetpoint = 127,
  .buttons = 0,
  .keypad = 0,
  .potmeter = 0,
  .switches1 = 0,
  .switches2 = 0,
  .switches3 = 0,
  .switches4 = 0
};



bool getRemoteSwitch(char button) {
  if((button >='0' && button <='9')|| button=='*' || button=='#'){ // check keypad buttons
    if(message.keypad == button) return true;
  }
  else if(button >=0 && button < 8) {
    if((message.switches1) & 1<<button) return true;
  }
  else if(button >=8 && button < 16) {
    if((message.switches2)  & 1<<(button-8)) return true;
  }
  else if(button >=16 && button < 24) {
    if((message.switches3)  & 1<<(button-16)) return true;
  }
  else if(button >=24 && button < 32) {
    if((message.switches4)  & 1<<(button-24)) return true;
  }
  return false;
}

void writeRelay(int relay,bool state){

    if(state) digitalWrite(relays[relay],LOW);
    else digitalWrite(relays[relay],HIGH);
}

void setup() {
  Serial.begin(115200);

  PORT.begin(RS485_BAUD);//RS485
  pinMode(22, OUTPUT);//RS485 S-R

  pinMode(LED_BUILTIN, OUTPUT);

  for (int i = 0; i < 16; i++) {
    pinMode(relays[i], OUTPUT);
    digitalWrite(relays[i], HIGH);
  }

  for (int n = 0; n < 6; n++) {
    eyeServo[n].attach(servoPins[n]);
    eyeServo[n].write(servoCenters[n]);
  }
#ifdef DEBUG
  Serial.println("started up");
#endif
}

void loop() {
  static unsigned long looptime = 0;
  static unsigned long blinktimer = 0;
  static bool blinking = 0;
  DynamixelPoll();

  if (millis() > looptime + 49) { // 20Hz
    looptime = millis();
    if (remoteTimeout < 100) remoteTimeout++;
    if (remoteTimeout > 20 && validMessage > 0) { // remote has timed out
      // STOP THE VEHICLE !!
#ifdef DEBUG
      Serial.println("no signal");
#endif
      validMessage = 0;
      for (int i = 0; i < 16; i++) {
        digitalWrite(relays[i], HIGH); // all 16 outputs off
      }
        for (int n = 0; n < 6; n++) {
    eyeServo[n].write(servoCenters[n]);
  }
    }
    //// now, here the magic should happen for switch-type actions
   for (int n = 0; n < NUM_ACTIONS; n++) {
     myActionList[n].update();
   }
   


  /// and now for a blink:
  #ifdef BLINK_KEY
  if (getRemoteSwitch(BLINK_KEY) && !blinking)  {
    blinking = 1;
    blinktimer = 10;
    eyeServo[5].write(servoMins[5]-20); // fixed working level
  }
  else if (blinking) {
    if (blinktimer > 0) blinktimer--;
    if (blinktimer == 0) {
      blinking = 0;
      eyeServo[5].write(servoCenters[5]); // fixed level
    }
  }
  #endif
  ///// and, the eye-servos feeding on the nunchuck (joystick) information:
  if(validMessage>0){
  if (message.buttons == 0) {
    #ifdef ANIMAL_LOVE
    eyeServo[0].write(map(message.xSetpoint, 255, 0, servoMins[0], servoMax[0]));
    eyeServo[1].write(map(message.xSetpoint, 255, 0, servoMins[1], servoMax[1]));
    eyeServo[2].write(map(message.ySetpoint, 0, 255, servoMins[2], servoMax[2]));
    eyeServo[3].write(map(message.ySetpoint, 255, 0, servoMins[3], servoMax[3]));
    #else
    eyeServo[0].write(map(message.xSetpoint, 0, 255, servoMins[0], servoMax[0]));
    eyeServo[1].write(map(message.xSetpoint, 255, 0, servoMins[1], servoMax[1]));
    eyeServo[2].write(map(message.ySetpoint, 0, 255, servoMins[2], servoMax[2]));
    eyeServo[3].write(map(message.ySetpoint, 255, 0, servoMins[3], servoMax[3]));
    #endif
  }
  //--------- then for the eyelids when the 2nd button is pressed --------------
  if (message.buttons == 64) {
    int leftLidvalue = constrain((127 + (message.ySetpoint - 127) + (message.xSetpoint - 127)),0,255);
    int rightLidvalue = constrain((127 + (message.ySetpoint - 127) - (message.xSetpoint - 127)),0,255);
    
    eyeServo[4].write(map(leftLidvalue, 0, 255, servoMins[4], servoMax[4])); // inverted servo position!! 
    #ifdef ANIMAL_LOVE
    if (!blinking) eyeServo[5].write(map(rightLidvalue, 0, 255, servoMax[5], servoMins[5]));
     #else
  if (!blinking) eyeServo[5].write(map(rightLidvalue, 0, 255, servoMins[5], servoMax[5]));
      #endif
   // Serial.print(leftLidvalue);
   // Serial.print(',');
   // Serial.println(rightLidvalue);
  }
  //-----------  DURING DRIVING (MOTOR ON  -------------
  if (message.buttons == 128 || message.buttons == 192) {
    #ifdef ANIMAL_LOVE
    eyeServo[0].write(map(message.xSetpoint, 255, 0, servoMins[0], servoMax[0]));
    eyeServo[1].write(map(message.xSetpoint, 255, 0, servoMins[1], servoMax[1]));
    #else
    eyeServo[0].write(map(127 + 2 * (message.xSetpoint - 127), 0, 255, servoMax[0], servoMins[0]));
    eyeServo[1].write(map(127 + 2 * (message.xSetpoint - 127), 0, 255, servoMax[1], servoMins[1]));
    #endif
    eyeServo[2].write(map(message.ySetpoint, 255, 0, servoMins[2], servoMax[2]));
    eyeServo[3].write(map(message.ySetpoint, 255, 0, servoMins[3], servoMax[3]));
  }
}
#ifdef DEBUG
  Serial.print('{');
  Serial.print(message.xSetpoint);
  Serial.print(',');
  Serial.print(message.ySetpoint);
  Serial.print(',');
  Serial.print(message.buttons);
  Serial.print(',');
  Serial.print(message.keypad);
  Serial.print(',');
  Serial.print(message.potmeter);
  Serial.print(',');
  Serial.print(message.switches1);
  Serial.print(',');
  Serial.print(message.switches2);
  Serial.print(',');
  Serial.print(message.switches3);
  Serial.print(',');
  Serial.print(message.switches4);
  Serial.println("},");
#endif
}
}


void ProcessDynamixelData(const unsigned char ID, const int dataLength, const unsigned char* const Data) {
  if (digitalRead(13))digitalWrite(13, LOW); else digitalWrite(13, HIGH);
  validMessage = 1;
  remoteTimeout = 0;
  if (Data[0] == 0x03) { //write
    message.xSetpoint = Data[1];
    message.ySetpoint = Data[2];
    message.buttons = Data[3];
    message.keypad = Data[4];
    message.potmeter = Data[5];
    message.switches1 = Data[6];
    message.switches2 = Data[7];
    message.switches3 = Data[8];
    message.switches4 = Data[9];
  }
}
