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
#include <Arduino.h> // the EarlPhilHower Arduino port of Pico SDK
#include <Wire.h>    // the I2C communication lib for the display, PCA9685 etc
#include <config.h>  // the specifics for the controlled robot or vehicle
#include <hardware/watchdog.h>
// hardware on every board: the relay sockets
#include <PicoRelay.h>
PicoRelay relay;
//////////////////////////////////////////////////////////////////////////////////////////////
// the one and only global channel array containing the received values from RF
// at present 14 of the 16 channels are used. Enter the save values (FAILSAFE) in these arrays
//                               0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
int channels[NUM_CHANNELS] =   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// for most of the exoot:        X  Y nb kp vo sw sw sw sw
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_OLED
// OLED display
#include <Adafruit_GFX.h>      // graphics, drawing functions (sprites, lines)
#include <Adafruit_SSD1306.h>  // display driver
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
void processScreen(int mode, int position);  // look at the bottom,
#endif
/////////// waveshare motor ///////////////
#ifdef USE_DDSM
#include <ddsm_ctrl.h>
#include <SoftwareSerial.h>
DDSM_CTRL dc;
SoftwareSerial DDSMport(18,19);
#endif
////////// waveshare ST3215 servo /////////
#ifdef USE_STS
#include <SCServo.h>
#include <SoftwareSerial.h>
SMS_STS st;
SoftwareSerial STSport(18,19);
#endif
// the USB joystick bit used by LUMI 
#ifdef USB_JOYSTICK
#include <Joystick.h>
#endif
//////// incremental encoder, might be populated on board
#ifdef USE_ENCODER
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
//// RS485, as passthrough of radio data, or as separate port
#ifdef USE_RS485
// for communication with motor driver and other externals
#include "RS485Reader.h"
#endif
// RS485 uses the same serial port and MAX485 driver as the Robotis Dynamixel. 
// They cannot be used in parallel
#ifdef ROBOTIS
  #include <Dynamixel2Arduino.h>
  #define DXL_SERIAL   Serial1
  #define DEBUG_SERIAL Serial
  const int DXL_DIR_PIN = 2; // DYNAMIXEL RS485 DIR PIN
  const uint8_t DXL_ID = 1;
  const float DXL_PROTOCOL_VERSION = 2.0;
  Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
  using namespace ControlTableItem;  //This namespace is required to use Control table item names
#endif

// running modes
#define IDLE 0
#define ACTIVE 1
#define PLAYBACK 2
/////// Electromen motor drivers (or other sigh-magnitude PWM drivers)
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
//// Action libray to select button press actions (sound + relay trigger)
#include "Action.h"  // needs audio and the available motor's to link actions to.
/// key to play standalone animation. Keeps playing when Remote is turned off
#ifdef ANIMATION_KEY
#include "Animation.h"
Animation animation(defaultAnimation, STEPS);
#endif
// for triggers or tracks on DFRobot players. Note: they have to be installed
// otherwise the initialisation will hang
#ifdef USE_AUDIO
DFRobot_DF1201S player1,player2;
SoftwareSerial player1port(7, 6);
SoftwareSerial player2port(17, 16);  //RX  TX ( so player TX, player RX)
void processAudio();
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
// radio communication: for now either the CRSF (ELRS) or the (old) APC220 434 MHz system
#ifdef USE_CRSF
#include "CRSF.h"
CRSF crsf;
#else
#include "Radio.h"
#endif
//////////////////////////////////////////////////////////////////////////////////////////
// and here the program starts
//////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
////////////////////////////////
#ifdef USE_DDSM
	DDSMport.begin(115200);
	dc.pSerial = &DDSMport;
	dc.set_ddsm_type(210); 	// config the type of ddsm. 
	dc.clear_ddsm_buffer(); // clear ddsm serial buffer.
	// change the ID of DDSM210.
	// args: ddsm_change_id(GOAL_ID)
	//dc.ddsm_change_id(4); // change the DDSM210 ID to 4
  dc.ddsm_change_mode(4, 2);  // 0 pwm, 2 speed, 3 position
  #endif
#ifdef USE_STS
  STSport.begin(1000000);
  st.pSerial = &STSport;
  // int ID_ChangeFrom = 1;
  // int ID_Changeto   = 16;
  // st.unLockEprom(ID_ChangeFrom);//unlock EPROM-SAFE
  // st.writeByte(ID_ChangeFrom, SMS_STS_ID, ID_Changeto);//ID
  // st.LockEprom(ID_Changeto);//EPROM-SAFE locked
#endif
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
relay.begin();
///////////////////
#ifdef USE_MOTOR
  motorLeft.init();
  motorRight.init();
  tandkrans.init();
#endif
// RS485 (dynamixel protocol) on Serial1:
#ifdef USE_RS485
  RS485Init(RS485_BAUD, RS485_SR);
#endif

#ifdef ROBOTIS
  Serial1.setTX(0);
  Serial1.setRX(1);
    // Set Port baudrate to 1000000bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(1000000);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  
  dxl.ping(DXL_ID);

  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(DXL_ID);
  dxl.setOperatingMode(DXL_ID, OP_POSITION);
  dxl.torqueOn(DXL_ID);

  // Limit the maximum velocity in Position Control Mode. Use 0 for Max speed
  dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID, 0);

#endif

  // radio on Serial2: CRSF or APC RF:
#ifdef USE_CRSF
  crsf.begin();
#else
  RFinit();
  RFsetSettings(2);
#endif
// and now start up the channel buffer!
  for (int n = 0; n < NUM_CHANNELS; n++) {
        channels[n] = saveValues[n];
  }
  watchdog_enable(200, 1);  // 100 ms timeout, pause_on_debug = true
}

void loop() {
  static int mode = IDLE;  // check the status
  static bool startedUp = false; // to avoid responding to the inital timeout (zero) 
// the following are important to make sure the brakes are switched on after a second of inactivity
  static bool brakeState = 1;
  static unsigned long brakeTimer;
// poll functions outside the 20Hz main loop
#ifndef USE_CRSF
  RadioPoll();
#endif
// RS485 as interface. 
#ifdef USE_RS485
  RS485Poll();
#endif
// there in an encoder on the board, optional
#ifdef USE_ENCODER
  pio_sm_exec_wait_blocking(pio, sm, pio_encode_in(pio_x, 32));
  int position = pio_sm_get_blocking(pio, sm);
#endif
// -----------------------------------------------------------------------------
// the 20 Hz main loop starts here!
// -----------------------------------------------------------------------------
  static unsigned long looptime;
  if (millis() > looptime + 19) {
    looptime = millis();

#ifdef USE_CRSF
    crsf.GetCrsfPacket();
    if(crsf.crsfData[1] == 24) startedUp = true; // so now we can respond to a timeout
    if (crsf.crsfData[1] == 24 && mode == ACTIVE) {
      if (digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN, LOW);
      else digitalWrite(LED_BUILTIN, HIGH);
      // in 16 channel mode the last two channels are used by ELRS for other things
      // check https://github.com/ExpressLRS/ExpressLRS/issues/2363
#ifdef ANIMATION_KEY     
      if(!animation.isPlaying()){
#else
      if(true){
#endif
        for (int n = 0; n < 16; n++) {
          channels[n] = constrain(map(crsf.channels[n], CRSF_CHANNEL_MIN-CRSF_CHANNEL_OFFSET, CRSF_CHANNEL_MAX-CRSF_CHANNEL_OFFSET, 0, 255),0,255);  //write
        }
      }
      // channels[8] contains the animation start and stop, so is always written by remote
      channels[8] = constrain(map(crsf.channels[8], CRSF_CHANNEL_MIN-CRSF_CHANNEL_OFFSET, CRSF_CHANNEL_MAX, 0, 255),0,255);  //write
      crsf.UpdateChannels();
    }
#endif
// now for specific interpretation of all the channels
#ifdef USE_DDSM
  dc.ddsm_ctrl(4, map(channels[1],0,255,-2100,2100), 0);
#endif

#ifdef USE_STS
// top yaw
st.WritePosEx(16, map(channels[0],0,255,1024,3072), 2000, 100);//servo(ID1) speed=1500，acc=50，move to position=2000.
// top pitch
st.WritePosEx(15, map(channels[1],0,255,512,2048), 2000, 100);//servo(ID1) speed=1500，acc=50，move to position=2000.
// elbow pitch
st.WritePosEx(14, map(channels[2],0,255,2700,1024), 1000, 20);//servo(ID1) speed=1500，acc=50，move to position=2000.
// double joint pitch, joint 12 leads, joint 13 has been tuned to follow
int centerpos = 1875;
int value = map(channels[2],0,255,-625,625); 
int offset = 300;
st.WritePosEx(13, centerpos - value + offset, 1000, 20);
st.WritePosEx(12, centerpos + value, 1000, 20);
// bottom yaw
st.WritePosEx(11, map(channels[3],0,255,1024,3072), 1000, 20);
#endif


#ifdef LUMI
    // Now, LUMI uses some special function to control the remote - perhaps the other relays can become actions
    // TODO (channel 12 as switch point to check the relays)
    // map the joystick input to the relay switches, only when the first switch is on
    if(getRemoteSwitch(0)) relay.joystickToRelays(channels[0],channels[1]);
    // and now for audio control
    //    processAudio(); // as separate void below...   
#endif
// THe DC motors are controlled with sign-magnitude (PWM functions in the motor class)
// only when there is no animation playing (no driving while animating)
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
// now, the following is for debug but also for recording motion tracks. The typical motion track
// for animal love or for animaltroniek contains 9 channels. All channels 16 (max, see NUM_CHANNELS)
#ifdef DEBUG
#define PRINT_CHANNELS 9
Serial.print('{');
    for (int i = 0; i < PRINT_CHANNELS; i++) {
      Serial.print(channels[i]);
      if (i < (PRINT_CHANNELS-1)) Serial.print(',');
    }
    Serial.println("},");
#endif
// a special key (typically in last key/switch buffer) is the one for starting and stopping animations
// needless to say, this key value should NOT! be recorded
#ifdef ANIMATION_KEY
if(getRemoteSwitch(ANIMATION_KEY) && !animation.isPlaying()){
  animation.start();      
  motorLeft.setSpeed(0,brakeState);
  motorRight.setSpeed(0,brakeState);
}
if (animation.isPlaying() && !getRemoteSwitch(ANIMATION_KEY)) animation.stop();
#endif
// RS485 passthrough of Remote data (for eyes, etc). In order to reduce the data load
// the BUFFER_PASSTHROUGH can be set to the minimum number of bytes necessary
// the Dynamixel protocol uses an ID in the message which has to be set here
#ifdef USE_RS485
   #ifdef BUFFER_PASSTHROUGH
    unsigned char headMessage[BUFFER_PASSTHROUGH];
    for (int i = 0; i < BUFFER_PASSTHROUGH; i++) {
      headMessage[i] = channels[i];  // transparent pass-through
    }
    RS485WriteBuffer(13, headMessage, BUFFER_PASSTHROUGH);  // check ID!!
    #else 
    #ifdef EXPERIMENT
  
    RS485WriteByte(0, 1, channels[2]);
    #endif
    #endif

#endif

#ifdef ROBOTIS
  dxl.setGoalPosition(DXL_ID, map(channels[2],0,255,1024,3072));
#endif

// now for the important mode / time-out settings related to CRSF communication. 
// only start up when a valid message has been received (see top of the loop)
// go to safe state after 9 timeout steps (so 0.5 sec)
// startup when the timeout has been reset (equals 0)
#ifdef USE_CRSF
    if (crsf.getTimeOut() > 9 && mode == ACTIVE) {
#else
    if (getTimeOut() > 9 && mode == ACTIVE) {
#endif
      mode = IDLE;
      digitalWrite(LED_BUILTIN, HIGH);
#ifdef ANIMATION_KEY
      if(!animation.isPlaying() ){
#else
        if(true){
#endif
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
      if(startedUp) mode = ACTIVE;
    }
// this is where the mapping to Relays and sounds takes place
#ifdef NUM_ACTIONS
    for (int n = 0; n < NUM_ACTIONS; n++) {
      myActionList[n].update();
    }
#endif
/////////// kick the time out checker! //////////
#ifdef USE_CRSF
    crsf.nudgeTimeOut();
#else
    nudgeTimeOut();
#endif
// and the motor bit: timer for the brakeState
#ifdef USE_MOTOR
  if(brakeTimer > 0) {brakeTimer --;brakeState = 0;}
  if(brakeTimer == 0) brakeState = 1;
#endif
// important: when an animation is playing (is checked in the animation class)
#ifdef ANIMATION_KEY
  animation.update();
#endif
// now the RF processing
  watchdog_update();
  }  
// ------------------------------------------------------------------
// the end of the 20Hz loop
// ------------------------------------------------------------------
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
// 
// now on the other core we run the USB joystick bit AND the audio controllers
void setup1(){
  #ifdef USB_JOYSTICK
  Joystick.begin(); 
#endif
  // audio players
#ifdef USE_AUDIO
audioInit(&player1, &player1port, &player2, &player2port);
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
    #ifdef LUMI
        processAudio(); // as separate void below...   
    #endif
  }
}
// end of core1 code
//--------------------------------------------------------------------------------
// the following function is called when RS485 data is received. This is currently
// not in use
#ifdef USE_RS485
void ProcessRS485Data(int ID, int dataLength, unsigned char *Data) {
}
#endif
// for the other type of radio (not CRSF)
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
// and now for the display function, a number of possible menu visualisations (for now set to menu 1)
#ifdef USE_OLED
void processScreen(int mode, int position) {
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
#ifdef ANIMATION_KEY
    if (animation.isPlaying()){ 
      display.print (F("anim run"));
    }
    else display.print (F("anim stop"));
#endif
    // print bars
    for (int n = 0; n < NUM_CHANNELS-2; n++) {
      display.fillRect(n * 6, 32 - channels[n] / 8, 4, 32, SSD1306_INVERSE);
    }
    display.fillRect(124, 0, 4, position, SSD1306_WHITE);
  } else if (menu == 2) {
    display.setCursor(0, 0);  // Start at top-left corner
    display.setTextSize(1);   // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.println(F("1234567890 actions"));
    #ifdef NUM_ACTIONS
    for (int i = 0; i < NUM_ACTIONS; i++) {
      if (myActionList[i].getState() == 1) display.fillRect(i * 6, 9, 5, 5, SSD1306_INVERSE);
      else display.drawRect(i * 6, 9, 5, 5, SSD1306_INVERSE);
    }
    #endif
  }
  display.display();
}
#endif
// and the audio processing for a.o. Lumi. 
// current issue is that checking player1.isPlaying() waits for milliseconds, so cannot be used to check file status
// also starting up a file takes more than 50 mS (so you will see a dip in message rate)
// it would be advisable to run this on the other core, were it not for the fact that that would conflict
// with the USB joystick functionality. 
#ifdef USE_AUDIO
void processAudio(void){
  static int isPlaying = 0;
     static int volume1;
     //static int playTimer = 0; 
     int trackToPlay = channels[13]/8;
     if(trackToPlay == 0 && isPlaying && channels[6]<100){
       player1.pause();
       isPlaying = 0;
     }
     else if (trackToPlay >0 && trackToPlay < (NUM_TRACKS+1) && trackToPlay !=isPlaying && channels[6]<100){
       player1.playSpecFile(tracklist[trackToPlay-1]);
       //player1.playFileNum(trackToPlay);
       isPlaying = trackToPlay;
     }
     if(channels[4]!=volume1) player1.setVol(map(channels[4],0,255,0,32));
     volume1 = channels[4];
     // the separate samples:
     static int playingSample;
     static int volume2;
     if(channels[7]!=volume2) player2.setVol(map(channels[7],0,255,0,32));
     volume2 = channels[7];
     
     if(channels[9]>(127+30) && (playingSample !=2)){
      playingSample = 2;
      player2.playFileNum(1);
      // player2.playSpecFile("/mp3/02-ang.mp3");
      //       Serial.print("file: ");
      // Serial.println(player2.getCurFileNumber());
      //playTimer = 20;
     }
     else if (channels[9]<(127-30) && (playingSample !=5)){
      player2.playFileNum(6);
      // player2.playSpecFile("/mp3/05-noo.mp3");
      // Serial.print("file: ");
      // Serial.println(player2.getCurFileNumber());
      playingSample = 5;
      //playTimer = 20;
     }
     else if ((channels[11]&16) && (playingSample != 3)){
      //player2.playSpecFile("/mp3/03-slp.mp3");
      playingSample = 3; 
      player2.playFileNum(3);
      //       Serial.print("file: ");
      // Serial.println(player2.getCurFileNumber());
      //playTimer = 20;
     }
     else if ((channels[11]&64) && (playingSample != 6)){
      //player2.playSpecFile("/mp3/06-yes.mp3");
      playingSample = 6; 
      player2.playFileNum(7);
      //       Serial.print("file: ");
      // Serial.println(player2.getCurFileNumber());
      //playTimer = 20;
     }
     else if ((channels[0]<100) && (playingSample != 4)){
      // player2.playSpecFile("/mp3/04-mov.mp3");
      //       Serial.print("file: ");
      // Serial.println(player2.getCurFileNumber());
      player2.playFileNum(4);
      playingSample = 4; 
      //playTimer = 20;
     }   
      else if (((channels[12]&16) || (channels[12]&32)) && (playingSample != 1)){
        player2.playFileNum(8);
      // player2.playSpecFile("/mp3/01-alm.mp3");
      //       Serial.print("file: ");
      // Serial.println(player2.getCurFileNumber());
      playingSample = 1; 
      //playTimer = 20;
     }
     //else playingSample = 0;
      //if(playTimer>0) playTimer --; 
      //if(playTimer ==0) playingSample = 0;  
}
#endif