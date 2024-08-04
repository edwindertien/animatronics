// this controlbox can use multiple radio systems. 
// on th RF spot there is place for an
// APC220. using RX and Tx
// link RF-set to gnd and write 'RD' or 'WR'
// with a specific configuration string
// for BetaFPV/ELRS/CrossFire you can send and 
// receive a CRSF string. make sure remote and receiver
// are paired (see betafpv manual)




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
// important radio communication materials
#include "Radio.h"
#define NUM_CHANNELS 17
unsigned char channels[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
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
  if (channels[16] & 16 && button == 'a') return true;
  else if (channels[16] & 32 && button == 'b') return true;
  else return false;
}
// important mapping of actions, buttons, relay channels and sounds
Action myActionList[] = {
  Action('a', 0, TOGGLE, "/bubble.mp3", &player1),
  Action('b', 1, DIRECT, "/jaws.mp3", &player1),
  Action('b', 2, TOGGLE, "", &player1),
  Action('d', 3, TOGGLE, "", &player1),
  Action('e', 4, TOGGLE, "", &player1),
  Action('f', 5, TOGGLE, "", &player1),
  Action('g', 6, TOGGLE, "", &player1),
  Action('h', 7, TOGGLE, "", &player1)
};
// and here the program starts
void writeRelay(int relay, bool state) {
  if (relay >= 0 && relay < 16 && !state) {
    pwm.setPWM(relay, 0, 4095);
  } else if (relay >= 0 && relay < 16 && state) {
    pwm.setPWM(relay, 0, 0);
  }
}


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
  audioInit(&player1,&player1port,&player2,&player2port);
  // radio
  RFinit();
  RFsetSettings(2);
// dynamixel
  Serial1.begin(57600);
  Serial1.setTX(0);
  Serial1.setRX(1);
  pinMode(RS485_SR, OUTPUT);
}



void loop() {
  static unsigned long looptime;
  static int mode;
// poll functions outside the 20Hz main loop
  RadioPoll();
  DynamixelPoll();
  pio_sm_exec_wait_blocking(pio, sm, pio_encode_in(pio_x, 32));
  int position = pio_sm_get_blocking(pio, sm);
// the 20 Hz main loop
  if (millis() > looptime + 49) {
    looptime = millis();
    if (getTimeOut() > 9 && mode == ACTIVE) {
      mode = IDLE;
      digitalWrite(LED_BUILTIN, HIGH);
      for (int n = 0; n < NUM_CHANNELS; n++) {
        channels[n] = saveValues[n];
      }
    }
    else if (getTimeOut() < 1 && mode == IDLE) {
      mode = ACTIVE;
    }
    for (int n = 0; n < 8; n++) {
      myActionList[n].update();
    }
    processScreen(mode,position);
    nudgeTimeOut();
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
    static int menu;
     button = !digitalRead(PUSH_BUTTON);
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
