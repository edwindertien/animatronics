// this controlbox can use multiple radio systems. 
// on th RF spot there is place for an
// APC220. using RX and Tx
// link RF-set to gnd and write 'RD' or 'WR'
// with a specific configuration string
// for BetaFPV/ELRS/CrossFire you can send and 
// receive a CRSF string. make sure remote and receiver
// are paired (see betafpv manual)

// resources:
// https://arduino-pico.readthedocs.io/en/latest/index.html

#define USE_CRSF (1)
//#define USE_AUDIO (1)
#define MAX_SPEED 100
#include <Arduino.h>
#include <Wire.h>  // the I2C communication lib for the display
// OLED display
#include <Adafruit_GFX.h>      // graphics, drawing functions (sprites, lines)
#include <Adafruit_SSD1306.h>  // display driver
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
void processScreen(int mode, int position); // look at the bottom, 
// PCA9685 pwm driver for 16 (relay) channels

#include "Adafruit_PWMServoDriver.h"
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// encoder knob
#include <hardware/pio.h>
#include "quadrature.pio.h"
#define QUADRATURE_A_PIN 20
#define QUADRATURE_B_PIN 21
PIO pio = pio0;
unsigned int sm = pio_claim_unused_sm(pio, true);
#define PUSH_BUTTON 22
// for communication with motor driver and other externals
#include "DynamixelReader.h"

int dynamixelIDs [] =       {   40,  21,   42,   43,   44,    45,   46};
int dynamixelMaxPos [] =    {  260, 510, 2300, 1800, 1000,  1800, 1524}; 
int dynamixelMinPos [] =    {  460, 340, 1900, 2200, 3000,  2200, 2596}; 
int dynamixelOffset [] =    {    0,   0,    0,    0,    0,     0,    0};

// important radio communication materials
#include "Radio.h"
//#define NUM_CHANNELS 16
//unsigned char channels[NUM_CHANNELS] =   { 127, 127, 0, 0, 0, 0, 0, 0 };
//unsigned char saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0 };

#define NUM_CHANNELS 16
unsigned char channels[NUM_CHANNELS] =   { 127, 127, 0, 127, 0, 128, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char saveValues[NUM_CHANNELS] = { 127, 127, 0, 127, 0, 128, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// Audio: two DFRobot players on softserial uart
#include "Audio.h"
DFRobot_DF1201S player1, player2;
SoftwareSerial player1port(7, 6);
SoftwareSerial player2port(17, 16);  //RX  TX ( so player TX, player RX)
// running modes
#define IDLE 0
#define ACTIVE 1
#define PLAYBACK 2
// button actions, samples
#include "Action.h"
// matching function between keypad/button register and call-back check from action list
bool getRemoteSwitch(char button) {


  for(int i = 0; i< 8; i++){
    if(((channels[4]/127)+2*(abs(2-channels[5]/64))+8*(abs(2-channels[6]/64)) + 32*(channels[7]/127)) & 1<<i && button == 'a'+i) return true; 
    if(channels[5]<10) {digitalWrite(21,HIGH);digitalWrite(22,LOW);}
    else if(channels[5]>250) {digitalWrite(21,LOW);digitalWrite(22,HIGH);}
    else {digitalWrite(21,LOW);digitalWrite(22,LOW);}

  } 
  return false;
}
// important mapping of actions, buttons, relay channels and sounds
Action myActionList[] = {
  Action('a', 0, DIRECT, "/bubble.mp3", &player1),
  Action('b', 1, DIRECT, "/jaws.mp3", &player1),
  Action('c', 2, DIRECT, "", &player1),
  Action('d', 3, DIRECT, "", &player1),
  Action('e', 4, DIRECT, "", &player1),
  Action('f', 5, DIRECT, "", &player1),
  Action('g', 6, DIRECT, "", &player1),
  Action('h', 7, DIRECT, "", &player1)
};
// and here the program starts
void writeRelay(int relay, bool state) {
  if (relay >= 0 && relay < 16 && !state) {
    pwm.setPWM(relay, 0, 4095);
  } else if (relay >= 0 && relay < 16 && state) {
    pwm.setPWM(relay, 0, 0);
  }
}

#ifdef USE_CRSF
#include "CRSF.h"
CRSF crsf;
#else

#endif

#include <Motor.h>

Motor motorLeft(18, 19, 20);  // Motor 1 (Pins 18, 19 for direction and 20 for PWM)
Motor motorRight(26, 27, 28); // 
void setup() {
  Serial.begin(115200);
  // The display uses a standard I2C, on I2C 0, so no changes or pin-assignments necessary
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32
  display.clearDisplay();                     // start the screen
  pinMode(LED_BUILTIN, OUTPUT);
  // encoder
  pinMode(PUSH_BUTTON, INPUT_PULLUP);
  pinMode(QUADRATURE_A_PIN, INPUT_PULLUP);
  pinMode(QUADRATURE_B_PIN, INPUT_PULLUP);
  unsigned int offset = pio_add_program(pio, &quadratureA_program);
  quadratureA_program_init(pio, sm, offset, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
  // relay / servo
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(16000);
  for (int n = 0; n < 16; n++) { writeRelay(n, LOW); }
  // audio players
 #ifdef USE_AUDIO
  audioInit(&player1,&player1port,&player2,&player2port);
 #endif 

pinMode(22,OUTPUT);
pinMode(21,OUTPUT);

// dynamixel
  Serial1.begin(1000000);
  Serial1.setTX(0);
  Serial1.setRX(1);
  pinMode(RS485_SR, OUTPUT);

   for (int n = 0; n < 2; n++) {
    DynamixelWrite(dynamixelIDs[n], 28, 0); // 28 -> gain
    DynamixelWrite(dynamixelIDs[n], 29, 0); // 28 -> gain
    DynamixelWrite(dynamixelIDs[n], 16, 0); // (1 for data, 0 for silence)
  
  }

   for (int n = 2; n < 7; n++) {
    DynamixelWrite(dynamixelIDs[n], 28, 0); // 28 -> gain

    DynamixelWrite(dynamixelIDs[n], 16, 0); // (1 for data, 0 for silence)
  
  }
    // radio on Serial2
  #ifdef USE_CRSF
  crsf.begin();
  #else
  RFinit();
  RFsetSettings(2);
  #endif
}



void loop() {
  static unsigned long looptime;
  static int mode;
  static unsigned char headMessage [8];
// poll functions outside the 20Hz main loop
#ifndef USE_CRSF
  RadioPoll();
#endif
  DynamixelPoll();
  pio_sm_exec_wait_blocking(pio, sm, pio_encode_in(pio_x, 32));
  int position = pio_sm_get_blocking(pio, sm);
// the 20 Hz main loop
  if (millis() > looptime + 49) {
    looptime = millis();
#ifdef USE_CRSF
    crsf.GetCrsfPacket(); 
if (crsf.crsfData[1] == 24 && mode==ACTIVE) {
        for (int n = 0; n < 8; n++) {
      channels[n] =map(crsf.channels[n],CRSF_CHANNEL_MIN,CRSF_CHANNEL_MAX,0,255);  //write
//     motorLeft.setSpeed(map(channels[1],0,255,-255,255));
//     motorRight.setSpeed(map(channels[1],0,255,-255,255));
     motorLeft.setSpeed(getLeftValueFromCrossMix(map(channels[1],0,255,-MAX_SPEED,MAX_SPEED),map(channels[0],0,255,-MAX_SPEED,MAX_SPEED)));
     motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1],0,255,-MAX_SPEED,MAX_SPEED),map(channels[0],0,255,-MAX_SPEED,MAX_SPEED)));

    }
    crsf.UpdateChannels();


      
      
      DynamixelWrite(dynamixelIDs[0], 28, map(channels[2],0,255,0,8));
      DynamixelWrite(dynamixelIDs[1], 28, map(channels[2],0,255,0,8));
      DynamixelWrite(dynamixelIDs[2], 28, map(channels[2],0,255,0,8));

   DynamixelWrite(dynamixelIDs[0], 30, map(channels[0], 0, 255, dynamixelMinPos[0]-dynamixelOffset[0], dynamixelMaxPos[0]-dynamixelOffset[0]));
 DynamixelWrite(dynamixelIDs[1], 30, map(channels[1], 0, 255, dynamixelMinPos[1]-dynamixelOffset[1], dynamixelMaxPos[1]-dynamixelOffset[1]));

 DynamixelWrite(dynamixelIDs[2], 30, map(channels[1], 0, 255, dynamixelMinPos[2]-dynamixelOffset[2], dynamixelMaxPos[2]-dynamixelOffset[2]));


}
#endif
   for (int n = 0; n < 2; n++) {
    DynamixelWrite(dynamixelIDs[n], 34, map(channels[15],0,255,0,1023)); // 28 -> gain

  
  }
for(int i = 2; i<7; i++){
DynamixelWrite(dynamixelIDs[i], 28, map(channels[15],0,255,0,8));
}


DynamixelWrite(dynamixelIDs[0], 30, map(channels[4]-(127-channels[3]), 127, 255, dynamixelMinPos[0]-dynamixelOffset[0], dynamixelMaxPos[0]-dynamixelOffset[0]));
DynamixelWrite(dynamixelIDs[1], 30, map(channels[4]+(127-channels[3]), 127, 255, dynamixelMinPos[1]-dynamixelOffset[1], dynamixelMaxPos[1]-dynamixelOffset[1]));

DynamixelWrite(dynamixelIDs[2], 30, map(channels[5]-(127-channels[6]), 0, 255, dynamixelMinPos[2]-dynamixelOffset[2], dynamixelMaxPos[2]-dynamixelOffset[2]));
DynamixelWrite(dynamixelIDs[3], 30, map(channels[5]+(127-channels[6]), 0, 255, dynamixelMinPos[3]-dynamixelOffset[3], dynamixelMaxPos[3]-dynamixelOffset[3]));

DynamixelWrite(dynamixelIDs[4], 30, map(channels[0], 0, 255, dynamixelMinPos[4]-dynamixelOffset[4], dynamixelMaxPos[4]-dynamixelOffset[4]));

DynamixelWrite(dynamixelIDs[5], 30, map(channels[1], 0, 255, dynamixelMinPos[5]-dynamixelOffset[5], dynamixelMaxPos[5]-dynamixelOffset[5]));
DynamixelWrite(dynamixelIDs[6], 30, map(channels[1], 0, 255, dynamixelMinPos[6]-dynamixelOffset[6], dynamixelMaxPos[6]-dynamixelOffset[6]));


for(int i = 0; i<8; i++){
  headMessage[i] = channels[i+8];

}
headMessage[0] = channels[2];
DynamixelWriteBuffer(10, headMessage, 8);
//DynamixelWrite(dynamixelIDs[2], 28, map(channels[2],0,255,0,8));
#ifdef USE_CRSF
    if (crsf.getTimeOut() > 9 && mode == ACTIVE) {
#else
if (getTimeOut() > 9 && mode == ACTIVE) {
#endif
      mode = IDLE;
      digitalWrite(LED_BUILTIN, HIGH);
      for (int n = 0; n < NUM_CHANNELS; n++) {
        channels[n] = saveValues[n];
      }
      motorLeft.setSpeed(0);
      motorRight.setSpeed(0);
    }
#ifdef USE_CRSF
    else if (crsf.getTimeOut() < 1 && mode == IDLE) {
  #else
else if (getTimeOut() < 1 && mode == IDLE) {
  #endif
      mode = ACTIVE;
    }

    for (int n = 0; n < 8; n++) {
      myActionList[n].update();
    }
    processScreen(mode,position);
    #ifdef USE_CRSF
    crsf.nudgeTimeOut();
    #else
    nudgeTimeOut();
    #endif
  }
}

void ProcessRadioData(int ID, int dataLength, unsigned char *Data) {
  if (digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN, LOW);
  else digitalWrite(LED_BUILTIN, HIGH);
  if (Data[0] == 0x03) {
    for (int n = 0; n < NUM_CHANNELS; n++) {
      channels[n] = Data[n + 1];  //write
    }
  }
}
void ProcessDynamixelData(int ID, int dataLength, unsigned char *Data) {
}

void processScreen(int mode, int position){
// menu and button variable
    static bool button, oldbutton;
    static int menu = 1;
  //   button = !digitalRead(PUSH_BUTTON);
    if (button && !oldbutton) menu++;
    if (menu > 2) menu = 0;
    oldbutton = button;
/// and now for the display
    display.clearDisplay();
    if (menu == 0) {
      display.fillRect(0, 0, 4, position, SSD1306_WHITE);
      display.setTextSize(1);               // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);  // Draw white text
      display.setCursor(10, 0);             // Start at top-left corner
      if (mode == ACTIVE) display.println(F("ACTIVE"));
      if (mode == IDLE) display.println(F("IDLE"));
    } else if (menu == 1) {
      for (int n = 0; n < NUM_CHANNELS; n++) {
        display.setCursor(0, 0);  // Start at top-left corner
        display.setTextSize(1);   // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.println(F("1234567890 received"));
        display.fillRect(n * 6, 32 - channels[n] / 8, 4, 32, SSD1306_INVERSE);
      }
    } else if (menu == 2) {
      display.setCursor(0, 0);  // Start at top-left corner
      display.setTextSize(1);   // Draw 2X-scale text
      display.setTextColor(SSD1306_WHITE);
      display.println(F("1234567890 actions"));
      for (int i = 0; i < 8; i++) {
        if (myActionList[i].getState() == 1) display.fillRect(i * 6, 9, 5, 5, SSD1306_INVERSE);
        else display.drawRect(i * 6, 9, 5, 5, SSD1306_INVERSE);
      }
    }
    display.display();
}
