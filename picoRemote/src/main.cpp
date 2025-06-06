/////////////////////////////////////
// Universal Remote 
// using CRSF and/or APC220 RF transmitter

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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include <Wire.h>  // the I2C communication lib for the display and other things
#include "config.h"
//#define DEBUG (1)
// the total channels (inputs) for the system to work with currently set at 32
// channels consist of [array of mux values 16][array of chuck values 3][keypad 1][arrays of switches 4[array of dmx values 8]  
#define NUM_CHANNELS 32
unsigned int channels[NUM_CHANNELS] =   { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
// only used for MIDI
unsigned int buttons = 0;

#ifdef USE_CRSF
#include "crsf.h"
#endif

// OLED display
#ifdef USE_OLED
#include <Adafruit_GFX.h>      // graphics, drawing functions (sprites, lines)
#include <Adafruit_SSD1306.h>  // display driver
Adafruit_SSD1306 display = Adafruit_SSD1306(128, DISPLAY_HEIGHT, &Wire1);
void processScreen(int mode, int position, float battery); // look at the bottom, 
#endif

// NUNCHUCK
#ifdef USE_NUNCHUCK
// not sure where this library comes from
#include "nunchuck.h"
nunchuck chuck;
// calibration data for a specific nunchuck: 
#define X_CENTER 130
#define Y_CENTER 127
#endif

// for using USB_host
#ifdef USE_USB_MIDI
#include "USBhostfunctions.h"
#endif

// and for using DMX input channels
#ifdef USE_DMX
#include "DmxInput.h"
DmxInput dmxInput;
#define DMX_START_CHANNEL 1
#define DMX_NUM_CHANNELS 16
#define DMX_OFFSET 8
volatile uint8_t buffer[DMXINPUT_BUFFER_SIZE(DMX_START_CHANNEL, DMX_NUM_CHANNELS)];
#endif

// data used for the multiplexer. This is fixed at 16 channels
#include "muxcontrol.h"
int muxpins[] = {16,17,18,19};
MuxControl mux(muxpins, 4);

#ifdef USE_APC
#include "apcrf.h"
apcRF myTransmitter(22);
#endif

#ifdef USE_KEYPAD
#include <keypad.h>
// Define keypad pins
//                   C1 C2 C3  R1  R2  R3  R4
int keypadPins[] = { 2, 3, 11, 12, 13, 14, 15 };
keypad mypad(keypadPins, 7);
#endif

#ifdef USE_MAX17048
#include "Adafruit_MAX1704X.h"
Adafruit_MAX17048 maxlipo;
#endif

#ifdef LUMI
#define NUM_TRACKS 3
String tracklist[NUM_TRACKS] = {
  "arrival.mp3",
  "farewell.mp3",
  "moving.mp3",
};

#define NUM_SAMPLES 8
String samplelist[NUM_SAMPLES] = {
  "yes.mp3",
  "no.mp3",
  "wortel.mp3",
  "appel.mp3",
    "huh.mp3",
  "why.mp3",
  "grrrr.mp3",
  "alarm.mp3"
};
#endif


void setup() {
  // for debug
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

#ifdef USE_CRSF
  crsfInit();        // CRSF radio on Serial1
#endif

#ifdef USE_NUNCHUCK
  // nunchuck on I2C, sharing bus with Display
  chuck.begin(); // send the initilization handshake
#endif
  // simple analog mux on A0, controlled by pins [16..19]
  mux.initMux();

#ifdef USE_DMX
  // dmx on Serial 1 (GPIO 0 input)
  dmxInput.begin(0, DMX_START_CHANNEL, DMX_NUM_CHANNELS);  
  dmxInput.read_async(buffer);  // no-wait code
#endif

#ifdef USE_OLED
  // The display uses a standard I2C, on I2C 0, so no changes or pin-assignments necessary
  // In the constructor #Wire1 has been passed
  Wire1.setSCL(7);
  Wire1.setSDA(6);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32
  display.clearDisplay();                     // start the screen
#endif
  //
#ifdef USE_KEYPAD
  mypad.begin();  // Initialize the keypad
#endif
#ifdef USE_MAX17048
while (!maxlipo.begin()) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    delay(2000);
  }
  Serial.print(F("Found MAX17048"));
  Serial.print(F(" with Chip ID: 0x")); 
  Serial.println(maxlipo.getChipID(), HEX);
#endif
}




void loop() {  
#ifdef USE_CRSF
  crsfCallback(); // polling, timing is inside the callback
#endif    
// the 20 Hz main loop, all sampling into channels[] array, with transmission at the end.
static unsigned long looptime;
  if (millis() > looptime + 49) {
    looptime = millis();
    if(digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN,LOW); else digitalWrite(LED_BUILTIN,HIGH);
#ifdef USE_NUNCHUCK
    chuck.update(200);
#endif
    // get analog channels from mux
    for(int i = 0; i<16;i++){
      channels[i] = mux.checkMux(i);
      if(invertChannel[i]>0) channels[i] = 255-channels[i];
    }
 
// get channels from WiiNunchuck
#ifdef USE_NUNCHUCK
  if (chuck.buttons > 1){ // 2 of 3
    channels[16] = 127+(map(chuck.analogStickX,23,215,0,255)- X_CENTER)/(FAST_MODE);
    channels[17] = 127+(map(chuck.analogStickY,29,224,0,255)-Y_CENTER)/(FAST_MODE);
  }
  else {
    channels[16] = 127+(map(chuck.analogStickX,23,215,0,255)- X_CENTER)/(SLOW_MODE);
    channels[17] = 127+(map(chuck.analogStickY,29,224,0,255)-Y_CENTER)/(SLOW_MODE);
  }
    channels[18] = chuck.buttons * 64;
#endif
// get channels from DMX  
#ifdef USE_DMX 
        if(millis() > 100+dmxInput.latest_packet_timestamp()) {
       for(int i = 0; i<8; i++){
            channels[i+23] = 0;
          }
          channels[23] = 127;  // default values for ALAN
          channels[24] = 20;
          channels[25] = 64;
        }
        else {
          for(int i = 0; i<8; i++){
            channels[i+19] = buffer[i+1+DMX_OFFSET];
          }
    }
#endif
#ifdef USE_KEYPAD
    unsigned int keyValue = mypad.getKeypad();
    channels[19] = mypad.getKeyValue(keyValue);   
#endif
    // send to robot (choose your channels)
 
 #ifdef STUFF_SWITCHES 
     unsigned long switchbuffer = 0; 

    for (int i =0; i<16; i++){
      if(switchChannel[i]>0){
        if (channels[i]<64) switchbuffer += 1<<(2*(i));
        if (channels[i]>180) switchbuffer += 1<<(2*(i)+1);
      }
    }
    channels[20] = (switchbuffer) & 0x000000FF;
    channels[21] = (switchbuffer>>8) & 0x000000FF;
    channels[22] = (switchbuffer>>16) & 0x000000FF;
    channels[23] = (switchbuffer>>24) & 0x000000FF;

#endif
#ifdef DEBUG
for(int i = 0; i< 4; i++){
  Serial.print(channels[20+i]);
  if(i<3)Serial.print(',');
}
Serial.println("");
#endif
#ifdef USE_CRSF
    for(int i = 0; i<CRSF_MAX_CHANNEL; i++){
       rcChannels[i] = map(channels[channelMap[i]],0,255,CRSF_CHANNEL_MIN,CRSF_CHANNEL_MAX);
     }
#else
    // RobotWrite(13,channels[0],channels[1],channels[16],channels[17],channels[18],channels[19],channels[20],channels[21]);
    static unsigned char message [RF_MAX_CHANNEL];
    for(int i = 0; i< RF_MAX_CHANNEL; i++){
      message[i] = channels[channelMap[i]];
    }
    myTransmitter.RobotWrite(13,message,16);
#endif
  } // end of looptime (20Hz loop)

#ifdef USE_OLED
  // process the display
  static unsigned long screentime;
  if(millis()>screentime+99){
    screentime = millis();
#ifdef USE_MAX17048
  float cellVoltage = maxlipo.cellVoltage();
  if (isnan(cellVoltage)) {
    Serial.println("Failed to read cell voltage, check battery is connected!");
    delay(2000);
    return;
  }
  processScreen(1,4,maxlipo.cellPercent()); 
  #else
processScreen(1,4,0.0); 
#endif

    
  }
#endif
} // end of main loop


//////////////////////////
// USB handling on Core1
/////////////////////////

// core1's setup
void setup1() {
  // while (!Serial) {
  //  delay(100);   // wait for native usb
  // }
 #ifdef USE_USB_MIDI
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
  #endif
}

// core1's loop
void loop1(){ 
  #ifdef USE_USB_MIDI
  USBHost.task();
  #endif
}
////////////////// to be moved to libraries: 

void processScreen(int menu, int position, float battery){
    display.clearDisplay();
    if (menu == 0) {
      display.fillRect(0, 0, 4, position, SSD1306_WHITE);
      display.setTextSize(1.5);               // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);  // Draw white text
      display.setCursor(10, 0);             // Start at top-left corner
    } else if (menu == 1) {
      for (int n = 0; n < NUM_CHANNELS; n++) {
        display.setCursor(0, 0);  // Start at top-left corner
        display.setTextSize(1);   // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        if((char)channels[19] != ' ') display.print((char)channels[19]);
        if(battery>0.0) {display.setCursor(0,33);display.print("battery ");display.print(battery,2);display.print(" %");}

        display.fillRect(n * 5, 32 - channels[n] / 8, 4, channels[n] / 8, SSD1306_INVERSE);
      }
      if(channels[4]>10) {
          int samplenr = constrain(map(channels[4],0,255,0,NUM_SAMPLES),0,NUM_SAMPLES-1);
          display.fillRect(0,47,6*samplelist[samplenr].length()+2,10,SSD1306_WHITE);
          display.setTextColor(SSD1306_BLACK);
          display.setCursor(1,48);
          display.print(samplelist[samplenr]);}
      if(channels[6]>10) {
          int tracknr = constrain(map(channels[6],0,255,0,NUM_TRACKS),0,NUM_TRACKS-1);
          display.fillRect(64,47,6*tracklist[tracknr].length()+2,10,SSD1306_WHITE);
          display.setTextColor(SSD1306_BLACK);
          display.setCursor(64,48);
          display.print(tracklist[tracknr]);}
    } else if (menu == 2) {
      display.setCursor(0, 0);  // Start at top-left corner
      display.setTextSize(1);   // Draw 2X-scale text
      display.setTextColor(SSD1306_WHITE);
#ifdef USE_NUNCHUCK
    display.drawCircle(64, 32, 14, SSD1306_WHITE);
    display.drawLine(64, 18, 64, 46, SSD1306_WHITE);
    display.drawLine(50, 32, 78, 32, SSD1306_WHITE);
    display.fillCircle(map(channels[17], 0, 255, 50, 78), map(channels[18], 0, 255, 18,46), 3+channels[19]/64, SSD1306_WHITE);
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
