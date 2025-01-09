/******************************************************************
 * This source uses the picoServo board and RPI PICO W
 * it functions as USB-MIDI device, using 
 * I2C sensors / M5 potentiometer array and OLED for feedback
 * 
 * It uses the following libraries:
 * - M5Angle8
 * - usb_midi_host (from rppicomidi) + Adafruit TinyUSB
 * - Adafruit unified sensor + BNO055
 * - U8g2 library for OLED
 */


/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/* This sketch is enumerated as USB MIDI device. 
 * Following library is required
 * - MIDI Library by Forty Seven Effects
 *   https://github.com/FortySevenEffects/arduino_midi_library
 */
//    FILE: M5ANGLE8_analogRead8.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo (works well with build in plotter)
//     URL: https://github.com/RobTillaart/M5ANGLE8
/*********************************************************************
 MIT license, check LICENSE for more information
 Copyright (c) 2023 rppicomidi
 Modified from device_info.ino from 
 https://github.com/sekigon-gonnoc/Pico-PIO-USB/blob/main/examples/arduino/device_info/device_info.ino
 2nd Copyright notice below included per license terms.
*********************************************************************/

/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <Arduino.h>
#include <MIDI.h>
#include "Adafruit_TinyUSB.h"
// USB MIDI object
Adafruit_USBD_MIDI usb_midi;
// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);
/////////// for the M5 angle sensor on I2C port 0 //////////////////
#include "m5rotate8.h"
M5ROTATE8 MM;

/////////// function prototypes //////////////////
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);



void setup() {
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  //Serial.begin(115200);  // not useful when using USB MIDI - it is not a Teensy :)
  pinMode(LED_BUILTIN, OUTPUT);
  // THE FOLLOWING LINE CAUSES THE SYSTEM TO NOT RUN: 
  // usb_midi.setStringDescriptor("TinyUSB MIDI");

  // Initialize MIDI, and listen to all MIDI channels
  // This will also call usb_midi's begin()
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  // Attach the handleNoteOn function to the MIDI Library. It will
  // be called whenever the Bluefruit receives MIDI Note On messages.
  MIDI.setHandleNoteOn(handleNoteOn);

  // Do the same for MIDI Note Off messages.
  MIDI.setHandleNoteOff(handleNoteOff);
  Wire.setSDA(4);  // not necessary as these are the defaults... 
  Wire.setSCL(5);
  Wire.begin();
 MM.begin();
  MM.resetAll();
  
}
volatile int value[12];  // used to output AND print
volatile int key[8];
void loop() {
  static unsigned long looptime;
  static int oldvalue[12];
  static int oldkey[8];
#ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
#endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }
  if (millis() > looptime + 9) {
    looptime = millis();
    Serial.println(value[0]);
    for (int i = 0; i < 8; i++) {
      value[i] = MM.getAbsCounter(i);
      if(value[i]>127) value[i]=127;
      if(value[i]<0) value[i]=0;
      delayMicroseconds(100);
      key[i] = MM.getKeyPressed(i);

      if (value[i] != oldvalue[i]) {
        if(digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN,LOW); else digitalWrite(LED_BUILTIN,HIGH);
        MIDI.sendControlChange(i+1, value[i], 1);
      }
      if(key[i]!=oldkey[i]) {
        if(digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN,LOW); else digitalWrite(LED_BUILTIN,HIGH);
        if(key[i]>0) MIDI.sendNoteOn(i+36,127,1); else MIDI.sendNoteOff(i+36,0,1);
      }

      oldvalue[i] = value[i];
      oldkey[i] = key[i];
    }
  }
  // read any new MIDI messages
  MIDI.read();
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  // Log when a note is pressed.
  Serial.print("Note on: channel = ");
  Serial.print(channel);

  Serial.print(" pitch = ");
  Serial.print(pitch);

  Serial.print(" velocity = ");
  Serial.println(velocity);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  // Log when a note is released.
  Serial.print("Note off: channel = ");
  Serial.print(channel);

  Serial.print(" pitch = ");
  Serial.print(pitch);

  Serial.print(" velocity = ");
  Serial.println(velocity);
}
