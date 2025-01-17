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
// The usb midi host functions from rppicomidi are used under the following license: 
// MIT license, check LICENSE for more information
// Copyright (c) 2023 rppicomidi
// Modified from device_info.ino from 
// https://github.com/sekigon-gonnoc/Pico-PIO-USB/
// 
/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
//#define DEBUG (1)
//#define USE_CRSF (1)
//#define USE_DMX (1)
//#define USE_NUNCHUCK (1)
#include <Arduino.h>
int limit(int number,int min, int max){
  return  (((number)>(max))?(max):( ((number)<(min))?(min):(number)));}
// important radio communication materials
#define NUM_CHANNELS 32
#define TRANSMIT 16
unsigned char channels[NUM_CHANNELS] =   { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int buttons = 0;

#ifndef USE_DMX
#include "USBhostfunctions.h"
#endif

#include <Wire.h>  // the I2C communication lib for the display
// OLED display
#include <Adafruit_GFX.h>      // graphics, drawing functions (sprites, lines)
#include <Adafruit_SSD1306.h>  // display driver
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
void processScreen(int mode, int position); // look at the bottom, 

#define IDLE 0
#define ACTIVE 1


#ifdef USE_NUNCHUCK
// not sure where this library comes from
#include "nunchuck.h"
nunchuck chuck;
#define X_CENTER 130
#define Y_CENTER 127
#endif

#ifdef USE_DMX
#include "DmxInput.h"
DmxInput dmxInput;
#define DMX_START_CHANNEL 1
#define DMX_NUM_CHANNELS 16
#define DMX_OFFSET 8
volatile uint8_t buffer[DMXINPUT_BUFFER_SIZE(DMX_START_CHANNEL, DMX_NUM_CHANNELS)];
#endif


int muxpins[] = {16,17,18,19};
int usedChannel[]   = {0,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1};  // used channels on the mux
int switchChannel[] = {0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0};  // switch type channels
int invertChannel[] = {0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,1};  // values that need to be inverted

void initMux(){
  for(int i = 0; i<4; i++){pinMode(muxpins[i],OUTPUT);}
}
int checkMux(int channel){
  for(int i = 0; i<4; i++){
    if(channel & (1<<i)) {digitalWrite(muxpins[i],HIGH);} else {digitalWrite(muxpins[i],LOW);}
  }
  if(usedChannel[channel]){
    if(switchChannel[channel]){ 
      if(analogRead(A0)>100) return 0; else return 4095;
    }
    else {
      if(invertChannel[channel]) return(4095-analogRead(A0));
      else return analogRead(A0);
    }
  }
  else return 0;
}

void RobotWrite(int board, unsigned char *message, int messagelength);
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
  #ifdef USE_NUNCHUCK
  // nunchuck on I2C, sharing with Display
  chuck.begin(); // send the initilization handshake
  #endif
  // simple analog mux on A0, controlled by pins [16..19]
  initMux();
  #ifdef USE_DMX
  // dmx on Serial 1 (GPIO 0 input)
  dmxInput.begin(0, DMX_START_CHANNEL, DMX_NUM_CHANNELS);  
  dmxInput.read_async(buffer);  // no-wait code
  #endif
}

void loop() {
  static unsigned long looptime;
  static int mode;
  static int screentimer;

    
// the 20 Hz main loop
  if (millis() > looptime + 49) {
    looptime = millis();
    #ifdef USE_NUNCHUCK
    chuck.update(200);
    #endif
// get analog channels from mux
  for(int i = 0; i<16;i++){
    channels[i] = checkMux(i)/4;
    }
// get channels from WiiNunchuck
#ifdef USE_NUNCHUCK
#ifdef ALAN
if(chuck.buttons == 0) {
    channels[0] = 127+(map(chuck.analogStickX,23,215,0,255)- X_CENTER)/(2);
    channels[1] = 127+(map(chuck.analogStickY,29,224,0,255)-Y_CENTER)/(2);
}
else if (chuck.buttons==2)
{
    channels[0] =  127+(map(chuck.analogStickX,23,215,0,255)- X_CENTER)/(1.5);
    channels[1] = 127+(map(chuck.analogStickY,29,224,0,255)-Y_CENTER)/(1.5);
}
else 
{
    channels[0] =  127+(map(chuck.analogStickX,23,215,0,255)- X_CENTER)/(1);
    channels[1] = 127+(map(chuck.analogStickY,29,224,0,255)-Y_CENTER)/(1);
}
#else
    channels[0] =  chuck.analogStickX;
    channels[1] =  chuck.analogStickY;
#endif

   channels[6] = chuck.buttons * 64;
#endif
   #ifdef DEBUG
   Serial.print(channels[0]);
   Serial.print(',');
   Serial.println(channels[1]);
   #endif

   #ifdef USE_DMX
// get channels from DMX    
        if(millis() > 100+dmxInput.latest_packet_timestamp()) {
       for(int i = 0; i<8; i++){
            channels[i+16] = 0;
          }
          channels[18] = 127;
          channels[16] = 20;
          channels[17] = 64;
        }
        else {
          for(int i = 0; i<8; i++){
            channels[i+16] = buffer[i+1+DMX_OFFSET];
          }
    }
    #endif
    // send to robot (choose your channels)
    
  static unsigned char message [16];
          message[0]=channels[9];
          message[1]=channels[10];
          message[2]=channels[3];
          message[3]=channels[7];
          message[4]=channels[8];
          message[5]=channels[13];
          message[6]=channels[12];
          message[7]=buttons;      //buttons (midi keys)
          message[8]=channels[14];
          message[9]=channels[15];
          message[10]=channels[2]; //nunchuckbuttons and joystickbutton 
          message[11]=channels[16]; // midi cc
          message[12]=channels[17];// midi cc
          message[13]=channels[18];// midi cc
          message[14]=channels[19];// midi cc
          message[15]=channels[20];// midi cc
          


    #ifdef ALAN
     RobotWrite(13,channels[0],channels[1],channels[16],channels[17],channels[18],channels[19],channels[20],channels[21]);
    #else
    RobotWrite(13,message,16);
    #endif
    // show on screen
    if(screentimer ==0) {processScreen(0,4); screentimer = 2;}
    if(screentimer>0) screentimer--;

  }
}
//////////////////////////
// USB handling on Core1
/////////////////////////
#ifndef USE_DMX
// core1's setup
void setup1() {
  //while (!Serial) {
  //  delay(100);   // wait for native usb
 // }
  Serial.printf("Core1 setup to run TinyUSB host with pio-usb\r\n");

  // Check for CPU frequency, must be multiple of 120Mhz for bit-banging USB
  uint32_t cpu_hz = clock_get_hz(clk_sys);
  if ( cpu_hz != 120000000UL && cpu_hz != 240000000UL ) {
    delay(2000);   // wait for native usb
    Serial.printf("Error: CPU Clock = %u, PIO USB require CPU clock must be multiple of 120 Mhz\r\n", cpu_hz);
    Serial.printf("Change your CPU Clock to either 120 or 240 Mhz in Menu->CPU Speed \r\n", cpu_hz);
    while(1) delay(1);
  }

  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = PIN_USB_HOST_DP;
 
 // Change the next line to #if 1 if the Pico-PIO-USB library version is 0.5.3 or older.
 #if 0
 #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  /* Need to swap PIOs so PIO code from CYW43 PIO SPI driver will fit */
  pio_cfg.pio_rx_num = 0;
  pio_cfg.pio_tx_num = 1;
 #endif /* ARDUINO_RASPBERRY_PI_PICO_W */
 #endif
 
  USBHost.configure_pio_usb(1, &pio_cfg);
#ifdef PIN_5V_EN
  pinMode(PIN_5V_EN, OUTPUT);
  digitalWrite(PIN_5V_EN, PIN_5V_EN_STATE);
#endif
  // Optionally, configure the buffer sizes here
  // The commented out code shows the default values
  // tuh_midih_define_limits(64, 64, 16);

  // run host stack on controller (rhport) 1
  // Note: For rp2040 pico-pio-usb, calling USBHost.begin() on core1 will have most of the
  // host bit-banging processing works done in core1 to free up core0 for other works
  USBHost.begin(1);
}

// core1's loop
void loop1()
{
  USBHost.task();
}
#endif
////////////////// to be moved to libraries: 

void processScreen(int mode, int position){
// menu and button variable
    static bool button, oldbutton;
    static int menu = 2;
   if(channels[4]>140) button = 1; else button = 0;
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
      for (int n = 16; n < NUM_CHANNELS; n++) {
        display.setCursor(0, 0);  // Start at top-left corner
        display.setTextSize(1);   // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        //display.println(F("1234567890 "));
        display.fillRect((n-16) * 5, 32 - channels[n] / 8, 4, channels[n] / 8, SSD1306_INVERSE);
      }
    } else if (menu == 2) {
      display.setCursor(0, 0);  // Start at top-left corner
      display.setTextSize(1);   // Draw 2X-scale text
      display.setTextColor(SSD1306_WHITE);
#ifdef USE_NUNCHUCK
   display.drawCircle(64, 32, 14, SSD1306_WHITE);
   display.drawLine(64, 18, 64, 46, SSD1306_WHITE);
   display.drawLine(50, 32, 78, 32, SSD1306_WHITE);
   display.fillCircle(map(channels[0], 0, 255, 50, 78), map(channels[1], 0, 255, 18,46), 3+channels[6]/64, SSD1306_WHITE);
#endif

    display.drawCircle(16, 16, 14, SSD1306_WHITE);
    display.drawLine(16, 2, 16, 30, SSD1306_WHITE);
    display.drawLine(2, 16, 30, 16, SSD1306_WHITE);
    display.fillCircle(map(channels[9], 0, 255, 2, 30), map(channels[10], 0, 255, 30, 2), 3+channels[2]/127, SSD1306_WHITE);

    display.drawCircle(112, 16, 14, SSD1306_WHITE);
    display.drawLine(112, 2, 112, 30, SSD1306_WHITE);
    display.drawLine(98, 16,126, 16, SSD1306_WHITE);
    display.fillCircle(map(channels[14], 0, 255, 98, 126), map(channels[15], 0, 255, 30, 2), 3+channels[4]/127, SSD1306_WHITE);

        display.drawCircle(26, 48, 14, SSD1306_WHITE);
        display.drawLine(26, 34, 26, 62, SSD1306_WHITE);
   display.drawLine(12, 48, 40, 48, SSD1306_WHITE);
    display.fillCircle(map(channels[7], 0, 255, 12, 40), map(channels[8], 0, 255, 62, 34), 3, SSD1306_WHITE);

        display.drawCircle(102, 48, 14, SSD1306_WHITE);
        display.drawLine(102, 34, 102, 62, SSD1306_WHITE);
   display.drawLine(88, 48, 116, 48, SSD1306_WHITE);
    display.fillCircle(map(channels[13], 0, 255, 88, 116), map(channels[12], 0, 255, 62, 34), 3, SSD1306_WHITE);


   display.fillRect(32, 32 - channels[3] / 8, 4, channels[3] / 8, SSD1306_INVERSE);
   display.fillRect(90, 32 - channels[5] / 8, 4, channels[5] / 8, SSD1306_INVERSE);

    }
    display.display();
}

void RobotWrite(int board, unsigned char *message, int messagelength) {
  unsigned char length = messagelength+2;
  int checksumbuffer = board + length + 0x03;
  for(int i = 0; i< messagelength; i++){
    checksumbuffer += message[i];
  }
  unsigned char checksum = ~(checksumbuffer);
  unsigned char buff[length + 4];
  buff[0] = 0xFF;
  buff[1] = 0xFF;
  buff[2] = (unsigned char) board;
  buff[3] = length;
  buff[4] = 0x03;
  for(int i = 5; i<length+3; i++){
    buff[i] = message[i-5];
  }
  buff[length+3] = checksum;
  
  RFWriteRaw(buff, length + 4);
}
/*
void RobotWrite(int board, unsigned char x, unsigned char y, unsigned char pal, unsigned char pat, unsigned char bns, unsigned char r, unsigned char g, unsigned char b) {
  unsigned char length = 10;
  unsigned char checksum = ~( board + length + 0x03 + x + y + r + g + b + pal + pat + bns);
  unsigned char buff[length + 4] = {
    0xFF, 0xFF, // Header
    (unsigned char) board, //ID
    length, // length
    0x03, // write
    x,
    y,
    pal,
    pat,
    bns,
    r,
    g,
    b,
    checksum
  };
  RFWriteRaw(buff, length + 4);
}*/

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
