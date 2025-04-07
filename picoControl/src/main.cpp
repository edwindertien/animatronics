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
#include <Arduino.h>
#include <Wire.h>    // the I2C communication lib for the display, PCA9685 etc
#include "config.h"  // the specifics for the controlled robot

#ifdef USE_OLED
// OLED display
#include <Adafruit_GFX.h>      // graphics, drawing functions (sprites, lines)
#include <Adafruit_SSD1306.h>  // display driver
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
void processScreen(int mode, int position);  // look at the bottom,
#endif

// PCA9685 pwm driver for 16 (relay) channels
#include "Adafruit_PWMServoDriver.h"
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
void writeRelay(int relay, bool state) {
  if (relay >= 0 && relay < 16 && !state) {
    pwm.setPWM(relay, 0, 4095);
  } else if (relay >= 0 && relay < 16 && state) {
    pwm.setPWM(relay, 0, 0);
  }
}

#ifdef USE_ENCODER
// encoder knob
#include <hardware/pio.h>
#include "quadrature.pio.h"
#define QUADRATURE_A_PIN 20
#define QUADRATURE_B_PIN 21
PIO pio = pio0;
unsigned int sm = pio_claim_unused_sm(pio, true);
#define PUSH_BUTTON 22
#endif

#ifdef USE_RS485
// for communication with motor driver and other externals
#include "DynamixelReader.h"
#define BUFFER_PASSTHROUGH 9
#endif

#ifdef USE_AUDIO
// Audio: two DFRobot players on softserial uart
#include "Audio.h"
DFRobot_DF1201S player1, player2;
SoftwareSerial player1port(7, 6);
SoftwareSerial player2port(17, 16);  //RX  TX ( so player TX, player RX)
#endif

// running modes
#define IDLE 0
#define ACTIVE 1
#define PLAYBACK 2

#ifdef USE_MOTOR
#include <Motor.h>
// left pin, right pin, pwm pin, brake relay pin. set unused pins to -1
Motor motorLeft(18, 19, 20, 6);   // Motor 1 (Pins 18, 19 for direction and 20 for PWM)
Motor motorRight(26, 27, 28, 7);  //
Motor tandkrans(21, 22, -1, -1);  // 21 and 22 for control, no PWM (motorcontroller set at fixed speed)
#endif

#include "Animation.h"
Animation animation(defaultAnimation, STEPS);

// button actions, samples
#include "Action.h"
// important mapping of actions, buttons, relay channels and sounds
#define NUM_ACTIONS 6
Action myActionList[NUM_ACTIONS] = {
//  Action('a', -1, DIRECT, &tandkrans, 100, "/bubble.mp3", &player1),
  Action('1', -1, DIRECT, &tandkrans, -100),
  Action('2', -1, DIRECT, &tandkrans, -100),
  Action('3', 0, DIRECT),
  Action('#', 1, DIRECT),
  Action(8, 2, DIRECT),
  Action(9, 3, DIRECT),
};
// matching function between keypad/button register and call-back check from action list
// currently using one button channel (characters '0' and higher)
// and 32 switch positions (in 4 bytes)
bool getRemoteSwitch(char button) {
  if(button >='0' || button=='*' || button=='#'){ // check keypad buttons
    if(channels[KEYPAD_CHANNEL] == button) return true;
  }
  else if(button >=0 && button < 8) {
    if((channels[SWITCH_CHANNEL]) & 1<<button) return true;
  }
  else if(button >=8 && button < 16) {
    if((channels[SWITCH_CHANNEL+1]) & 1<<(button-8)) return true;
  }
  else if(button >=16 && button < 24) {
    if((channels[SWITCH_CHANNEL+2]) & 1<<(button-16)) return true;
  }
  else if(button >=24 && button < 32) {
    if((channels[SWITCH_CHANNEL+3]) & 1<<(button-24)) return true;
  }
  return false;
}

// radio communication materials
#ifdef USE_CRSF
#include "CRSF.h"
CRSF crsf;
#else
#include "Radio.h"
#endif

// and here the program starts
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef USE_OLED
  // The display uses a standard I2C, on I2C 0, so no changes or pin-assignments necessary
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32
  display.clearDisplay();                     // start the screen
#endif
  // encoder
#ifdef USE_ENCODER
  pinMode(PUSH_BUTTON, INPUT_PULLUP);
  pinMode(QUADRATURE_A_PIN, INPUT_PULLUP);
  pinMode(QUADRATURE_B_PIN, INPUT_PULLUP);
  unsigned int offset = pio_add_program(pio, &quadratureA_program);
  quadratureA_program_init(pio, sm, offset, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
#endif
  // relay / servo
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(16000);
  for (int n = 0; n < 16; n++) { writeRelay(n, LOW); }
  // audio players
#ifdef USE_AUDIO
  audioInit(&player1, &player1port, &player2, &player2port);
#endif

#ifdef USE_MOTOR
  motorLeft.init();
  motorRight.init();
  tandkrans.init();
#endif
// RS485 (dynamixel protocol) on Serial1:
#ifdef USE_RS485
  DynamixelInit(57600, RS485_SR);
#endif

  // radio on Serial2: CRSF or APC RF:
#ifdef USE_CRSF
  crsf.begin();
#else
  RFinit();
  RFsetSettings(2);
#endif
//animation.start();
}

void loop() {
  static int mode;  // check the status
// poll functions outside the 20Hz main loop
#ifndef USE_CRSF
  RadioPoll();
#endif

#ifdef USE_RS485
  DynamixelPoll();
#endif

#ifdef USE_ENCODER
  pio_sm_exec_wait_blocking(pio, sm, pio_encode_in(pio_x, 32));
  int position = pio_sm_get_blocking(pio, sm);
#endif
  // the 20 Hz main loop
  static unsigned long looptime;
  if (millis() > looptime + 49) {
    looptime = millis();

    animation.update();
#ifdef USE_CRSF
    crsf.GetCrsfPacket();
    if (crsf.crsfData[1] == 24 && mode == ACTIVE) {
      // in 16 channel mode the last two channels are used by ELRS for other things
      // check https://github.com/ExpressLRS/ExpressLRS/issues/2363
      for (int n = 0; n < 14; n++) {
        channels[n] = map(crsf.channels[n], CRSF_CHANNEL_MIN-CRSF_CHANNEL_OFFSET, CRSF_CHANNEL_MAX-CRSF_CHANNEL_OFFSET, 0, 255);  //write
      }
      crsf.UpdateChannels();
    }
#endif

#ifdef USE_MOTOR
#ifdef USE_CROSS_MIXING
    motorLeft.setSpeed(getLeftValueFromCrossMix(map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED), map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED)));
    motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED), map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED)));
#else
    motorLeft.setSpeed(map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED) - map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED));
    motorRight.setSpeed(map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED) + map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED));
#endif
#endif

#ifdef DEBUG
Serial.print('{');
    for (int i = 0; i < 9; i++) {
      Serial.print(channels[i]);
      if (i < 8) Serial.print(',');
    }
    Serial.println("},");
#endif

// RS485 passthrough of Remote data (for eyes, etc)
#ifdef USE_RS485

    unsigned char headMessage[BUFFER_PASSTHROUGH];
    for (int i = 0; i < BUFFER_PASSTHROUGH; i++) {
      headMessage[i] = channels[i];  // transparent pass-through
    }
    DynamixelWriteBuffer(13, headMessage, BUFFER_PASSTHROUGH);  // check ID!!
#endif

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
      #ifdef USE_MOTOR
      motorLeft.setSpeed(0);
      motorRight.setSpeed(0);
      #endif
    }
#ifdef USE_CRSF
    else if (crsf.getTimeOut() < 1 && mode == IDLE) {
#else
    else if (getTimeOut() < 1 && mode == IDLE) {
#endif

      mode = ACTIVE;
    }
    ///// this is where the mapping to Relays and sounds takes place
    for (int n = 0; n < NUM_ACTIONS; n++) {
      myActionList[n].update();
    }
/////////// kick the time out checker! //////////
#ifdef USE_CRSF
    crsf.nudgeTimeOut();
#else
    nudgeTimeOut();
#endif
  }  // the end of the 20Hz loop
// finally, different timer: screen update
#ifdef USE_OLED
  unsigned long screentimer;
  if (millis() > screentimer + 99) {
    screentimer = millis();
#ifdef USE_ENCODER
    processScreen(mode, position);
#else
    processScreen(mode, 0);
#endif
  }
#endif
}  // end of main



#ifdef USE_RS485
void ProcessDynamixelData(int ID, int dataLength, unsigned char *Data) {
}
#endif

#ifndef USE_CRSF
void ProcessRadioData(int ID, int dataLength, unsigned char *Data) {
  if (digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN, LOW);
  else digitalWrite(LED_BUILTIN, HIGH);
  if (Data[0] == 0x03) {
    for (int n = 0; n < NUM_CHANNELS; n++) {
      channels[n] = Data[n + 1];  //write
    }
  }
}
#endif


#ifdef USE_OLED
void processScreen(int mode, int position) {
  /// and now for the display
  static int menu = 1;
  display.clearDisplay();
  if (menu == 0) {
    display.fillRect(124, 0, 4, position, SSD1306_WHITE);
    display.setTextSize(1);               // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);  // Draw white text
  } else if (menu == 1) {
    display.setTextSize(1);  // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);  // Start at top-left corner
    if (mode == ACTIVE) display.println(F("ACTIVE"));
    if (mode == IDLE) display.println(F("IDLE"));
    // print keypad char
    display.setCursor(50, 0);
    if (channels[KEYPAD_CHANNEL] > 1) display.print((char)(channels[KEYPAD_CHANNEL]));
    // print bars
    for (int n = 0; n < NUM_CHANNELS; n++) {
      display.fillRect(n * 6, 32 - channels[n] / 8, 4, 32, SSD1306_INVERSE);
    }
    display.fillRect(124, 0, 4, position, SSD1306_WHITE);
  } else if (menu == 2) {
    display.setCursor(0, 0);  // Start at top-left corner
    display.setTextSize(1);   // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.println(F("1234567890 actions"));
    for (int i = 0; i < NUM_ACTIONS; i++) {
      if (myActionList[i].getState() == 1) display.fillRect(i * 6, 9, 5, 5, SSD1306_INVERSE);
      else display.drawRect(i * 6, 9, 5, 5, SSD1306_INVERSE);
    }
  }
  display.display();
}
#endif