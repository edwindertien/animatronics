// this controlbox can use multiple radio systems.
// on th RF spot there is place for an
// APC220. using RX and Tx
// link RF-set to gnd and write 'RD' or 'WR'
// with a specific configuration string
// for BetaFPV/ELRS/CrossFire you can send and
// receive a CRSF string. make sure remote and receiver
// are paired (see betafpv manual), flashed with 
// firmware v3.4.3 or up (same for Tx and Tx). Set 
// packet rate to 333 (or100), switches to 16/2 ch and telemetry to 1:128
// 
// TODO: for the animaltroniek the air or driving edition have not been 
// added yet.
//
// resources:
// https://arduino-pico.readthedocs.io/en/latest/index.html
// edit the config.h to set the specifics for a used robot or vehicle
#include <Arduino.h>
#include <Wire.h>    // the I2C communication lib for the display, PCA9685 etc
// button actions, samples
#include "config.h"  // the specifics for the controlled robot or vehicle

//////////////////////////////////////////////////////////////////////////////////////////////
#define NUM_CHANNELS 16
// at present 14 of the 16 channels are used. Enter the save values (FAILSAFE) in these arrays
//                                           0    1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
int channels[NUM_CHANNELS] =   { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#ifdef LUMI
const int saveValues[NUM_CHANNELS] = { 127, 127, 127, 127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#else 
const int saveValues[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
//                                           X    Y nb kp vo sw sw sw sw
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_OLED
// OLED display
#include <Adafruit_GFX.h>      // graphics, drawing functions (sprites, lines)
#include <Adafruit_SSD1306.h>  // display driver
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
void processScreen(int mode, int position);  // look at the bottom,
#endif

#ifdef USB_JOYSTICK
#include <Joystick.h>
#endif

#ifdef USE_9685
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
#else 
#include "PCA9635.h"
PCA9635 pwm(0x70);
void writeRelay(int relay, bool state) {
  if (relay >= 0 && relay < 16 && !state) {
    //pwm.write1(relay, 255);
    pwm.setLedDriverMode(relay, PCA963X_LEDOFF);
  } else if (relay >= 0 && relay < 16 && state) {
    pwm.setLedDriverMode(relay, PCA963X_LEDON);
  }
}
#endif
const int driveRelays[]={
  //      
  0b00001000,
  0b00011000,
  0b00010000,
  0b00110000,
  0b00100000,
  0b00100001,
  0b00000001,
  0b00000011,
  0b00000010,
  0b00000110,
  0b00000100,
  0b00001100,
};

bool joystickActive = false;  // global or static variable

void joystickToRelays(int x, int y) {
  const int center = 127;
  const int enterThreshold = 60;
  const int exitThreshold = 40;

  int dx = x - center;
  int dy = y - center;
  int distance = sqrt(dx * dx + dy * dy);

  // State transition with hysteresis
  if (!joystickActive && distance > enterThreshold) {
    joystickActive = true;
  } else if (joystickActive && distance < exitThreshold) {
    joystickActive = false;
  }

  if (joystickActive) {
    int relayNumber = constrain((180 + 360.0 * (atan2(dx, dy) / 6.28)) / 30, 0, 11);
    for (int i = 0; i < 6; i++) {
      if (driveRelays[relayNumber] & (1 << i)) {
        writeRelay(i, HIGH);
      } else {
        writeRelay(i, LOW);
      }
    }
  } else {
    for (int i = 0; i < 6; i++) {
      writeRelay(i, LOW);
    }
  }
}



#ifdef USE_ENCODER
// encoder knob
#include <hardware/pio.h>
#include "quadrature.pio.h"
#ifdef BOARD_V1
#define QUADRATURE_A_PIN 20
#define QUADRATURE_B_PIN 21
#define PUSH_BUTTON 22
#else 
#define QUADRATURE_A_PIN 13
#define QUADRATURE_B_PIN 14
#define PUSH_BUTTON 15
#endif
PIO pio = pio0;
unsigned int sm = pio_claim_unused_sm(pio, true);
#endif

#ifdef USE_RS485
// for communication with motor driver and other externals
#include "DynamixelReader.h"
#define BUFFER_PASSTHROUGH 9  // message size, reduce to relevant portion
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


// running modes
#define IDLE 0
#define ACTIVE 1
#define PLAYBACK 2

#ifdef USE_MOTOR
#define BRAKE_TIMEOUT 30 // in loops of 20Hz, so 1.5 sec
#include <Motor.h>
#ifdef BOARD_V1
// left pin, right pin, pwm pin, brake relay pin. set unused pins to -1
Motor motorLeft(20, 19, 18, 0);   // Motor 1 (Pins 18, 19 for direction and 20 for PWM)
Motor motorRight(28, 27, 26, 1);  //
Motor tandkrans(21, 22, -1, -1);  // 21 and 22 for control, no PWM (motorcontroller set at fixed speed)
# else
#ifdef ANIMAL_LOVE
Motor motorLeft(18, 19, 20, 1);   // Motor 1 (Pins 18, 19 for direction and 20 for PWM)
Motor motorRight(26, 27, 28, 2);  //
Motor tandkrans(21, 22, -1, -1);  // 26 and 27 for control, no PWM (motorcontroller set at fixed speed)
#else
Motor motorLeft(20, 19, 18, 0);   // Motor 1 (Pins 18, 19 for direction and 20 for PWM)
Motor motorRight(28, 21, 22, 1);  //
Motor tandkrans(26, 27, -1, -1);  // 26 and 27 for control, no PWM (motorcontroller set at fixed speed)
#endif
#endif
#endif


#include "Action.h"  // needs audio and the available motor's to link actions to.
#include "Animation.h"
Animation animation(defaultAnimation, STEPS);

#ifdef USE_AUDIO
DFRobot_DF1201S player1,player2;
SoftwareSerial player1port(7, 6);
SoftwareSerial player2port(17, 16);  //RX  TX ( so player TX, player RX)
#endif

// matching function between keypad/button register and call-back check from action list
// currently using one button channel (characters '0' and higher)
// and 32 switch positions (in 4 bytes)
bool getRemoteSwitch(char button) {

  if((button >='0' && button<='9') || button=='*' || button=='#'){ // check keypad buttons
    #ifdef KEYPAD_CHANNEL
    if(channels[KEYPAD_CHANNEL] == button) return true;
    #endif
  }
  #ifdef SWITCH_CHANNEL
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
  #endif
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

#ifdef USE_9685
  // relay / servo
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(16000);
  for (int n = 0; n < 16; n++) { writeRelay(n, LOW); }
  #else
  pwm.begin();
  for (int n = 0; n < 16; n++) { writeRelay(n, LOW); }
#endif
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
  DynamixelInit(RS_485_BAUD, RS485_SR);
#endif

  // radio on Serial2: CRSF or APC RF:
#ifdef USE_CRSF
  crsf.begin();
#else
  RFinit();
  RFsetSettings(2);
#endif

}

void loop() {
  static int mode;  // check the status
  static bool brakeState = 1;
  static unsigned long brakeTimer;
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
      if(!animation.isPlaying()){
        for (int n = 0; n < 14; n++) {
          channels[n] = constrain(map(crsf.channels[n], CRSF_CHANNEL_MIN-CRSF_CHANNEL_OFFSET, CRSF_CHANNEL_MAX-CRSF_CHANNEL_OFFSET, 0, 255),0,255);  //write
        }
      }
      // channels[8] contains the animation start and stop, so is always written by remote
      channels[8] = constrain(map(crsf.channels[8], CRSF_CHANNEL_MIN-CRSF_CHANNEL_OFFSET, CRSF_CHANNEL_MAX, 0, 255),0,255);  //write
      crsf.UpdateChannels();
    }
#endif

// Now, LUMI uses some special function to control the remote - perhaps the other relays can become actions
// TODO (channel 12 as switch point to check the relays)
#ifdef LUMI
    joystickToRelays(channels[0],channels[1]);

      //    if(channels[4]>10) {
      //     int samplenr = constrain(map(channels[4],0,255,0,NUM_SAMPLES),0,NUM_SAMPLES-1);
      //     display.fillRect(0,47,6*samplelist[samplenr].length()+2,10,SSD1306_WHITE);
      //     display.setTextColor(SSD1306_BLACK);
      //     display.setCursor(1,48);
      //     display.print(samplelist[samplenr]);}
      // if(channels[6]>10) {
      //     int tracknr = constrain(map(channels[6],0,255,0,NUM_TRACKS),0,NUM_TRACKS-1);
      //     display.fillRect(64,47,6*tracklist[tracknr].length()+2,10,SSD1306_WHITE);
      //     display.setTextColor(SSD1306_BLACK);
      //     display.setCursor(64,48);
      //     display.print(tracklist[tracknr]);}
    // next section moved to (std) using getButton
    // if(channels[12]&1<<0)writeRelay(10,HIGH); else writeRelay(10,LOW);
    // if(channels[12]&1<<4)writeRelay(9,HIGH); else writeRelay(9,LOW);
    // if(channels[12]&1<<5)writeRelay(8,HIGH); else writeRelay(8,LOW);
    // if(channels[12]&1<<6)writeRelay(11,HIGH); else writeRelay(11,LOW);
    // if(channels[12]&1<<2)writeRelay(6,HIGH); else writeRelay(6,LOW); // wheels out
    // if(channels[12]&1<<3)writeRelay(7,HIGH); else writeRelay(7,LOW); // wheels in
#endif

#ifdef USE_MOTOR
if(!animation.isPlaying()){
  #ifdef USE_SPEEDSCALING
  #ifdef USE_KEYPAD_SPEED
  if(getRemoteSwitch('#')){
  #else
  if(channels[2]==192){
  #endif
  brakeTimer = BRAKE_TIMEOUT;
  motorLeft.setSpeed(getLeftValueFromCrossMix(map(channels[1], 0, 255, -HIGH_SPEED, HIGH_SPEED), map(channels[0], 255, 0, -HIGH_SPEED, HIGH_SPEED)),brakeState);
  motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -HIGH_SPEED, HIGH_SPEED), map(channels[0], 255, 0, -HIGH_SPEED, HIGH_SPEED)),brakeState);
}
  else if (channels[2]==128){
    brakeTimer = BRAKE_TIMEOUT;
    motorLeft.setSpeed(getLeftValueFromCrossMix(map(channels[1], 0, 255, -LOW_SPEED, LOW_SPEED), map(channels[0], 255, 0, -LOW_SPEED, LOW_SPEED)),brakeState);
    motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -LOW_SPEED, LOW_SPEED), map(channels[0], 255, 0, -LOW_SPEED, LOW_SPEED)),brakeState);
  
  }
  else {
    motorLeft.setSpeed(0,brakeState);
    motorRight.setSpeed(0,brakeState);
  }
   #else
   brakeTimer = BRAKE_TIMEOUT;
   motorLeft.setSpeed(getLeftValueFromCrossMix(map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED), map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED)),brakeState);
   motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED), map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED)),brakeState);

   #endif
}
#endif

#ifdef DEBUG
Serial.print('{');
    for (int i = 0; i < 9; i++) {
      Serial.print(channels[i]);
      if (i < 8) Serial.print(',');
    }
    Serial.println("},");
#endif

#ifdef ANIMATION_KEY
if(getRemoteSwitch(ANIMATION_KEY) && !animation.isPlaying())animation.start();
if (animation.isPlaying() && !getRemoteSwitch(ANIMATION_KEY)) animation.stop();
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

      if(!animation.isPlaying() ){
      for (int n = 0; n < NUM_CHANNELS; n++) {
        channels[n] = saveValues[n];
      }
    }

      #ifdef USE_MOTOR
      motorLeft.setSpeed(0,brakeState);
      motorRight.setSpeed(0,brakeState);
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

#ifdef USE_MOTOR
  if(brakeTimer > 0) {brakeTimer --;brakeState = 0;}
  if(brakeTimer == 0) brakeState = 1;
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

void setup1(){
  #ifdef USB_JOYSTICK
  Joystick.begin(); 
#endif
}

void loop1(){
  static unsigned long looptime1;
  if(millis()>looptime1+49){
    looptime1=millis();
    #ifdef USB_JOYSTICK
    Joystick.X(map(channels[2],0,255,0,1023));
    Joystick.Y(map(channels[3],0,255,0,1023));
    Joystick.Z(map(channels[5],0,255,0,1023));
    Joystick.Zrotate(map(channels[9],0,255,0,1023));
  
    if(channels[11]&1<<4)Joystick.button(1,true); else Joystick.button(1,false);
    if(channels[11]&1<<6)Joystick.button(4,true); else Joystick.button(4,false);
    #endif
  }
}

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
    #ifdef KEYPAD_CHANNEL
    if (channels[KEYPAD_CHANNEL] > 1) display.print((char)(channels[KEYPAD_CHANNEL]));
    #endif
    display.setCursor(70, 0);
    if (animation.isPlaying()) display.print (F("anim run"));
    else display.print (F("anim stop"));
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