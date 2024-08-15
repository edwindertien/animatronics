/////////////////////////////////////
// Universal Remote 
// APC220 RF transmitter
// and nunchuck (no library)
// Original nunchuck:  signal:    ext. cable    black nunchuck:
// white -             ground     orange        white
// red -               3.3+v      black         red
// green -             data  A4   blue          blue
// yellow -            clock A5   red           green
//
//#define DEBUG (1)
// The following channel assignment is proposed
// 1 = 424
// 2 = 434
// 3 = 444
// 4 = 430
// 5 = 440
// resources:
// https://arduino-pico.readthedocs.io/en/latest/index.html
// Pico-DMX library   (c) 2021 Jostein Løwer SPDX-License-Identifier: BSD-3-Clause
// Use DMX in async mode (not waiting...)
// not sure where the WiiNunchuck.h library originates, but it IS the one that works
// with Pico, Teensy and Arduino
// 

//#define USE_CRSF (1)
#include <Arduino.h>
#include <Wire.h>  // the I2C communication lib for the display
// OLED display
#include <Adafruit_GFX.h>      // graphics, drawing functions (sprites, lines)
#include <Adafruit_SSD1306.h>  // display driver
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
void processScreen(int mode, int position); // look at the bottom, 

#define IDLE 0
#define ACTIVE 1

// important radio communication materials
#define NUM_CHANNELS 24
unsigned char channels[NUM_CHANNELS] =   { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// not sure where this library comes from
#include "nunchuck.h"
nunchuck chuck;

#include "DmxInput.h"
DmxInput dmxInput;
#define DMX_START_CHANNEL 1
#define DMX_NUM_CHANNELS 8
volatile uint8_t buffer[DMXINPUT_BUFFER_SIZE(DMX_START_CHANNEL, DMX_NUM_CHANNELS)];

int muxpins[] = {16,17,18,19};
int usedChannel[] = {0,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1};
int switchChannel[] =  {0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0};
void initMux(){
  for(int i = 0; i<4; i++){pinMode(muxpins[i],OUTPUT);}
}
int checkMux(int channel){
  for(int i = 0; i<4; i++){
    if(channel & (1<<i)) {digitalWrite(muxpins[i],HIGH);} else {digitalWrite(muxpins[i],LOW);}
  }
  if(usedChannel[channel]){
    if(switchChannel[channel]){ 
      if(analogRead(A0)>100) return 0; else return 1023;
    }
    else return analogRead(A0);
  }
  else return 0;
}

void RobotWrite(int board, unsigned char x, unsigned char y, unsigned char r, unsigned char v, unsigned char m);
void RFWriteRaw(unsigned char *buffer, int length);

void setup() {
  // for debug
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  // for RF
  Serial2.setRX(9);
  Serial2.setTX(8);
  Serial2.begin(9600);
  // The display uses a standard I2C, on I2C 0, so no changes or pin-assignments necessary
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32
  display.clearDisplay();                     // start the screen
  // nunchuck on I2C, sharing with Display
  chuck.begin(); // send the initilization handshake
  // simple analog mux on A0, controlled by pins [16..19]
  initMux();
  // dmx on Serial 1 (GPIO 0 input)
  dmxInput.begin(0, DMX_START_CHANNEL, DMX_NUM_CHANNELS);  
  dmxInput.read_async(buffer);  // no-wait code
}

void loop() {
  static unsigned long looptime;
  static int mode;
    
// the 20 Hz main loop
  if (millis() > looptime + 49) {
    looptime = millis();
    chuck.update(200);
// get analog channels from mux
  for(int i = 0; i<16;i++){
      channels[i] = checkMux(i)/4;
    }
// get channels from WiiNunchuck
    channels[0] = chuck.analogStickX;
    channels[1] = chuck.analogStickY;
    channels[6] = chuck.buttons * 64;
// get channels from DMX    
        if(millis() > 100+dmxInput.latest_packet_timestamp()) {
       for(int i = 0; i<8; i++){
            channels[i+16] = 0;
          }
        }
        else {
          for(int i = 0; i<8; i++){
            channels[i+16] = buffer[i+1];
          }
    }
    // send to robot (choose your channels)
    RobotWrite(13,channels[0],channels[1],channels[16],channels[17],channels[18]);
    // show on screen
    processScreen(0,4);
  }
}



void processScreen(int mode, int position){
// menu and button variable
    static bool button, oldbutton;
    static int menu;
     button = !digitalRead(22);
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
        //display.println(F("1234567890 "));
        display.fillRect(n * 5, 32 - channels[n] / 8, 4, channels[n] / 8, SSD1306_INVERSE);
      }
    } else if (menu == 2) {
      display.setCursor(0, 0);  // Start at top-left corner
      display.setTextSize(1);   // Draw 2X-scale text
      display.setTextColor(SSD1306_WHITE);
      display.println(F("actions"));

    }
    display.display();
}


void RobotWrite(int board, unsigned char x, unsigned char y, unsigned char r, unsigned char v, unsigned char m) {
  unsigned char length = 7;
  unsigned char checksum = ~( board + length + 0x03 + x + y + r + v + m);
  unsigned char buff[length + 4] = {
    0xFF, 0xFF, // Header
    (unsigned char) board, //ID
    length, // length
    0x03, // write
    x,
    y,
    r,
    v,
    m,
    checksum
  };
  RFWriteRaw(buff, length + 4);
}

/********************************************************************
   RAW SERIAL COMMUNICATION USING RS485 PROTOCOL

   RS485 uses an extra bit, being the Send/Receive bit. Therefore, we need
   'wrappers' around the Serial.XXX()'s that takes care of handling it.
   These functions are below.
 ********************************************************************/


// Writes the characters in buffer to the RS485. Blocks until the
// sending has finished.
void RFWriteRaw(unsigned char *buffer, int length) {
  //digitalWrite(RS485_SR, HIGH);
  if (digitalRead(LED_BUILTIN) == 0)digitalWrite(LED_BUILTIN, HIGH); else digitalWrite(LED_BUILTIN, LOW);
  Serial2.write((uint8_t*)buffer, length);
  Serial2.flush(); // waits for the buffer to be empty
  //digitalWrite(RS485_SR, LOW); // is necessary when we listen too (but we don't)
}