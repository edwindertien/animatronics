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
#define NUM_CHANNELS 17
unsigned char channels[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#include "nunchuck.h"
nunchuck chuck;



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


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(3,OUTPUT);
  digitalWrite(3,HIGH); // pullup source for I2C
  delay(50);
  Serial.begin(115200);
  // The display uses a standard I2C, on I2C 0, so no changes or pin-assignments necessary
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32
  display.clearDisplay();                     // start the screen

  chuck.begin(); // send the initilization handshake
  initMux();
}

void loop() {
  static unsigned long looptime;
  static int mode;

// the 20 Hz main loop
  if (millis() > looptime + 49) {
    looptime = millis();
    chuck.update(200);
    Serial.println(chuck.analogStickX);

  for(int i = 0; i<16;i++){
      channels[i] = checkMux(i)/4;
    }
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
        display.println(F("1234567890 "));
        display.fillRect(n * 6, 32 - channels[n] / 8, 4, channels[n] / 8, SSD1306_INVERSE);
      }
    } else if (menu == 2) {
      display.setCursor(0, 0);  // Start at top-left corner
      display.setTextSize(1);   // Draw 2X-scale text
      display.setTextColor(SSD1306_WHITE);
      display.println(F("actions"));

    }
    display.display();
}


