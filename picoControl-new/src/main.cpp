// Animatronic robot control — main.cpp
// Vehicle selection: set -DVEHICLENAME in platformio.ini build_flags
// Hardware capabilities: set feature flags in include/config.h (per vehicle block)
// Vehicle behaviour:     see src/platform_xxx.cpp

#include <Arduino.h>
#include "config.h"
#include "platform.h"
#include "PicoRelay.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Global channel array: received RF values, mapped to 0-255
//                          0    1    2  3  4  5  6  7  8  9 10 11 12 13 14 15
int channels[NUM_CHANNELS] = { 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//////////////////////////////////////////////////////////////////////////////////////////////

PicoRelay relay;

// Handshake flag: Core 1 waits for this before starting CRSF/audio
// so it never touches peripherals before Core 0 has initialised them.
volatile bool _core0SetupDone = false;

#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
void processScreen(int mode, int position);
#endif

#ifdef USE_DDSM
#include <ddsm_ctrl.h>
#include <SoftwareSerial.h>
DDSM_CTRL dc;
SoftwareSerial DDSMport(18, 19);
#endif

#ifdef USE_STS
#include <SCServo.h>
#include <SoftwareSerial.h>
SMS_STS st;
SoftwareSerial STSport(13, 12);  // BOARD V3.0
#endif

#ifdef USB_JOYSTICK
#include <Joystick.h>
#endif

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

#ifdef USE_RS485
#include "RS485Reader.h"
#define FAST_SPEED 200
#define SLOW_SPEED 150
#endif

#ifdef ROBOTIS
#include <Dynamixel2Arduino.h>
#define DXL_SERIAL   Serial1
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = 2;
const uint8_t DXL_ID = 1;
const float DXL_PROTOCOL_VERSION = 2.0;
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
using namespace ControlTableItem;
#endif

#ifdef USE_M5_SERVOS
#include "M5Unit8Servos.h"
M5Unit8Servos servos;
#endif

#ifdef CAN_DRIVER
#include "mcp_can.h"
#include "TMotor_ServoConnection.h"
#define CAN0_INT 2
#endif

#ifdef CUBEMARS
#define ID_MOTOR_1 104
MCP_CAN CAN0(&SPI1, 17);
TMotor_ServoConnection servo_conn(CAN0);
#endif

#define IDLE     0
#define ACTIVE   1
#define PLAYBACK 2

#include "Action.h"

#ifdef ANIMATION_KEY
#include "Animation.h"
Animation animation(defaultAnimation, DEFAULT_STEPS);
#ifdef EXPO_KEY
Animation expanimation(expoAnimation, EXPO_STEPS);
#endif
// Each platform that uses recorded animations defines ANIMATION_TRACK_H
// to the name of its track header file (set in config.h per vehicle block)
#ifdef ANIMATION_TRACK_H
#pragma message("Animation track: " ANIMATION_TRACK_H)
#include ANIMATION_TRACK_H
#endif
#endif // ANIMATION_KEY

#ifdef ANIMATION_KEY
  #define ANIM_PLAYING() animation.isPlaying()
#else
  #define ANIM_PLAYING() false
#endif

#if USE_AUDIO >= 1
DFRobot_DF1201S player1;
SoftwareSerial player1port(7, 6);
#endif
#if USE_AUDIO >= 2
DFRobot_DF1201S player2;
SoftwareSerial player2port(17, 16);
#endif

// ----------------------------------------------------------------------------
// getRemoteSwitch — maps button/switch identifiers to channel values
// ----------------------------------------------------------------------------
bool getRemoteSwitch(char button) {
    if ((button >= '0' && button <= '9') || button == '*' || button == '#') {
#ifdef KEYPAD_CHANNEL
        if (channels[KEYPAD_CHANNEL] == button) return true;
#endif
    }
#ifdef SWITCH_CHANNEL
    else if (button >= 0  && button < 8)  { if (channels[SWITCH_CHANNEL]   & 1 << button)       return true; }
    else if (button >= 8  && button < 16) { if (channels[SWITCH_CHANNEL+1] & 1 << (button-8))   return true; }
    else if (button >= 16 && button < 24) { if (channels[SWITCH_CHANNEL+2] & 1 << (button-16))  return true; }
    else if (button >= 24 && button < 32) { if (channels[SWITCH_CHANNEL+3] & 1 << (button-24))  return true; }
#endif
    return false;
}

#ifdef USE_CRSF
#include "core1_crsf.h"
#else
#include "Radio.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

#ifdef USE_DDSM
    DDSMport.begin(115200);
    dc.pSerial = &DDSMport;
    dc.set_ddsm_type(210);
    dc.clear_ddsm_buffer();
    dc.ddsm_change_mode(4, 2);
#endif

#ifdef USE_STS
    STSport.begin(1000000);
    st.pSerial = &STSport;
#endif

    // Audio is initialised on Core 1 via platformSetup1() to keep
    // SoftwareSerial bit-banging off Core 0 (avoids UART corruption at 420kbaud)

#ifdef USE_OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
#endif

#ifdef USE_ENCODER
    pinMode(PUSH_BUTTON, INPUT_PULLUP);
    pinMode(QUADRATURE_A_PIN, INPUT_PULLUP);
    pinMode(QUADRATURE_B_PIN, INPUT_PULLUP);
    unsigned int offset = pio_add_program(pio, &quadratureA_program);
    quadratureA_program_init(pio, sm, offset, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
#endif

#if defined(BOARD_V1) || defined(BOARD_V2)
    relay.begin();
#endif

#ifdef USE_MOTOR
    configureMotors();
#endif

#ifdef USE_RS485
    RS485Init(RS485_BAUD, RS485_SR);
#endif

#ifdef ROBOTIS
    Serial1.setTX(0);
    Serial1.setRX(1);
    dxl.begin(1000000);
    dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
    dxl.ping(DXL_ID);
    dxl.torqueOff(DXL_ID);
    dxl.setOperatingMode(DXL_ID, OP_POSITION);
    dxl.torqueOn(DXL_ID);
    dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID, 0);
#endif

#ifdef CAN_DRIVER
    while (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) != CAN_OK) {
        Serial.println("Error Initializing MCP2515...");
        delay(100);
    }
    Serial.println("MCP2515 Initialized Successfully!");
    CAN0.setMode(MCP_NORMAL);
#endif

#ifdef CUBEMARS
    servo_conn.set_origin(ID_MOTOR_1, 0);
#endif

#ifdef USE_CRSF
    // CRSF is initialised on Core 1 in setup1()
#else
    RFinit();
    RFsetSettings(2);
#endif

    for (int n = 0; n < NUM_CHANNELS; n++) channels[n] = saveValues[n];

#ifdef EXPO_KEY
    pinMode(EXPO_KEY, INPUT_PULLUP);
#endif

#ifdef USE_M5_SERVOS
    delay(500);
    servos.begin(Wire, 0x25);
#endif

    // --- vehicle-specific setup (sequences, RS485 motor init, etc.) ---
    platformSetup();

    // Signal Core 1 that all hardware is initialised and it is safe to start
    _core0SetupDone = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////////////////////////////////////////////////
bool brakeState = 1;           // file-scope so platform files can read it
unsigned long brakeTimer = 0;  // file-scope so platform files can reset it

void loop() {
    static int mode = IDLE;
    static bool startedUp = false;

    // Poll functions that run outside the 20Hz gate
#ifndef USE_CRSF
    RadioPoll();
#endif
#ifdef USE_RS485
    RS485Poll();
#endif
#ifdef USE_ENCODER
    pio_sm_exec_wait_blocking(pio, sm, pio_encode_in(pio_x, 32));
    int position = pio_sm_get_blocking(pio, sm);
#else
    int position = 0;
#endif

    // -------------------------------------------------------------------------
    // 20 Hz main loop
    // -------------------------------------------------------------------------
    static unsigned long looptime;
    if (millis() < looptime + 49) return;
    looptime = millis();

    // --- CRSF: read channels mapped by Core 1 ---
#ifdef USE_CRSF
    if (crsfReady()) {
        if (digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN, LOW);
        else                          digitalWrite(LED_BUILTIN, HIGH);
        int fresh[16];
        crsfGetChannels(fresh, 16);
#if defined(ANIMATION_KEY) && defined(EXPO_KEY)
        if (!animation.isPlaying() && !expanimation.isPlaying())
#endif
        {
            for (int n = 0; n < NUM_CHANNELS; n++) channels[n] = fresh[n];
        }
        channels[8] = fresh[8];
        if (!startedUp) startedUp = true;
    }
#endif

    // --- Specialist hardware output ---
#ifdef USE_DDSM
    dc.ddsm_ctrl(4, map(channels[1], 0, 255, -2100, 2100), 0);
#endif

#ifdef USE_STS
    st.WritePosEx(16, map(channels[0], 0, 255, 1024, 3072), 2000, 100);
    st.WritePosEx(20, map(channels[5], 0, 255, 0, 1048), map(channels[6], 0, 255, 0, 2048), 100);
    st.WritePosEx(15, map(channels[1], 0, 255, 512, 2048), 2000, 100);
    st.WritePosEx(14, map(channels[2], 0, 255, 2700, 1024), 1000, 20);
    { int cv = 1875, val = map(channels[2], 0, 255, -625, 625), off = 300;
      st.WritePosEx(13, cv - val + off, 1000, 20);
      st.WritePosEx(12, cv + val,       1000, 20); }
    st.WritePosEx(11, map(channels[3], 0, 255, 1024, 3072), 1000, 20);
#endif

    // --- DC motor drive (cross-mix) ---
#ifdef USE_MOTOR
#if defined(ANIMATION_KEY) && defined(EXPO_KEY)
    if (!animation.isPlaying() && !expanimation.isPlaying())
#endif
    {
#ifdef USE_SPEEDSCALING
        if (channels[2] == 192) {
            brakeTimer = BRAKE_TIMEOUT;
            motorLeft.setSpeed( getLeftValueFromCrossMix( map(channels[1], 0, 255, -HIGH_SPEED, HIGH_SPEED), map(channels[0], 255, 0, -HIGH_SPEED, HIGH_SPEED)), brakeState);
            motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -HIGH_SPEED, HIGH_SPEED), map(channels[0], 255, 0, -HIGH_SPEED, HIGH_SPEED)), brakeState);
        } else if (channels[2] == 128) {
            brakeTimer = BRAKE_TIMEOUT;
            motorLeft.setSpeed( getLeftValueFromCrossMix( map(channels[1], 0, 255, -LOW_SPEED, LOW_SPEED), map(channels[0], 255, 0, -LOW_SPEED, LOW_SPEED)), brakeState);
            motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -LOW_SPEED, LOW_SPEED), map(channels[0], 255, 0, -LOW_SPEED, LOW_SPEED)), brakeState);
        } else {
            motorLeft.setSpeed(0, brakeState);
            motorRight.setSpeed(0, brakeState);
        }
#else
        brakeTimer = BRAKE_TIMEOUT;
        motorLeft.setSpeed( getLeftValueFromCrossMix( map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED), map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED)), brakeState);
        motorRight.setSpeed(getRightValueFromCrossMix(map(channels[1], 0, 255, -MAX_SPEED, MAX_SPEED), map(channels[0], 255, 0, -MAX_SPEED, MAX_SPEED)), brakeState);
#endif
    }
#endif // USE_MOTOR

#ifdef ANIMATION_DEBUG
    Serial.print('{');
    for (int i = 0; i < 9; i++) { Serial.print(channels[i]); if (i < 8) Serial.print(','); }
    Serial.println("},");
#endif

    // --- Animation playback (recorded tracks) ---
#ifdef ANIMATION_KEY
    if (getRemoteSwitch(ANIMATION_KEY) && !animation.isPlaying()) {
        animation.start();
#ifdef USE_MOTOR
        motorLeft.setSpeed(0, brakeState);
        motorRight.setSpeed(0, brakeState);
#endif
    }
    if (animation.isPlaying() && !getRemoteSwitch(ANIMATION_KEY)) animation.stop();
#endif

#ifdef EXPO_KEY
    if (mode == IDLE) {
        if (!digitalRead(EXPO_KEY) && !expanimation.isPlaying()) expanimation.start();
        if (expanimation.isPlaying() && digitalRead(EXPO_KEY)) {
            expanimation.stop();
            motorLeft.setSpeed(0, brakeState);
            motorRight.setSpeed(0, brakeState);
            for (int n = 0; n < NUM_CHANNELS; n++) channels[n] = saveValues[n];
        }
        if (expanimation.isPaused() || !expanimation.isPlaying()) {
            motorLeft.setSpeed(0, brakeState);
            motorRight.setSpeed(0, brakeState);
        } else {
            motorLeft.setSpeed(-40, brakeState);
            motorRight.setSpeed(-40, brakeState);
        }
    }
#endif

    // --- Recorded animation update ---
    // Runs BEFORE RS485 so animation channel values are transmitted when playing
#ifdef ANIMATION_KEY
    animation.update();
#endif
#ifdef EXPO_KEY
    expanimation.update();
#endif

    // --- RS485 passthrough or vehicle steering ---
#ifdef USE_RS485
#ifdef BUFFER_PASSTHROUGH
    {
        unsigned char headMessage[BUFFER_PASSTHROUGH];
        for (int i = 0; i < BUFFER_PASSTHROUGH; i++) headMessage[i] = channels[i];
        RS485WriteBuffer(13, headMessage, BUFFER_PASSTHROUGH);

#ifdef RS485_DEBUG
        static unsigned long _rs485DumpTimer;
        if (millis() - _rs485DumpTimer > 1000) {
            _rs485DumpTimer = millis();
            Serial.print("RS485 TX: ");
            for (int i = 0; i < BUFFER_PASSTHROUGH; i++) {
                Serial.print(headMessage[i]);
                Serial.print(" ");
            }
            Serial.print("  [mode=");
            Serial.print(mode == ACTIVE ? "ACTV" : "IDLE");
            Serial.print(" rf=");
            Serial.print(crsfReady() ? "OK" : "NO");
            Serial.println("]");
        }
#endif
    }
#else
    // Steering and drive for RS485-connected motor controllers (AMI, SCUBA, etc.)
    if ((channels[0] > 140 || channels[0] < 100) && mode == ACTIVE && !ANIM_PLAYING())
        RS485WriteByte(22, 1, map(channels[0], 0, 255, 255, 0));
    else
        RS485WriteByte(22, 1, 127);

    if (channels[2] == 192) {
        if      (channels[1] < 110 && mode == ACTIVE && !ANIM_PLAYING())
            RS485WriteByte(18, 3, constrain(map(channels[1], 0, 110, FAST_SPEED, 0), 0, FAST_SPEED));
        else if (channels[1] > 135 && mode == ACTIVE && !ANIM_PLAYING())
            RS485WriteByte(18, 1, constrain(map(channels[1], 135, 255, 0, FAST_SPEED), 0, FAST_SPEED));
        else
            RS485WriteByte(18, 2, 0);
    } else if (channels[2] == 128) {
        if      (channels[1] < 110 && mode == ACTIVE && !ANIM_PLAYING())
            RS485WriteByte(18, 3, constrain(map(channels[1], 0, 110, SLOW_SPEED, 0), 0, SLOW_SPEED));
        else if (channels[1] > 135 && mode == ACTIVE && !ANIM_PLAYING())
            RS485WriteByte(18, 1, constrain(map(channels[1], 135, 255, 0, SLOW_SPEED), 0, SLOW_SPEED));
        else
            RS485WriteByte(18, 2, 0);
    } else {
        RS485WriteByte(18, 2, 0);
    }
#endif // BUFFER_PASSTHROUGH
#endif // USE_RS485

#ifdef ROBOTIS
    dxl.setGoalPosition(DXL_ID, map(channels[2], 0, 255, 1024, 3072));
#endif

#ifdef CUBEMARS
    servo_conn.set_pos_spd(ID_MOTOR_1, 180, 1000, 1000);
    if (!digitalRead(CAN0_INT)) {
        while (CAN_MSGAVAIL == CAN0.checkReceive()) servo_conn.can_receive();
    }
    servo_conn.print_motor_vars(ID_MOTOR_1);
#endif

    // --- Timeout / mode management ---
#ifdef USE_CRSF
    // Loss: crsfLost() is true when Core 1 confirmed 500ms silence.
    // We cannot use crsfReady() here — it's already false when LINK_LOST.
    if (crsfLost() && mode == ACTIVE) {
#else
    if (getTimeOut() > 9 && mode == ACTIVE) {
#endif
        mode = IDLE;
        digitalWrite(LED_BUILTIN, HIGH);
#if defined(ANIMATION_KEY) && defined(EXPO_KEY)
        if (!animation.isPlaying() && !expanimation.isPlaying())
#endif
        {
            for (int n = 0; n < NUM_CHANNELS; n++) {
#ifdef ANIMATION_KEY
                if (n == 8) continue;  // preserve animation key channel
#endif
                channels[n] = saveValues[n];
            }
        }
        platformOnIdle();
    }
#ifdef USE_CRSF
    // Recovery: crsfReady() becomes true again once Core 1 receives a good frame
    else if (crsfReady() && mode == IDLE) {
#else
    else if (getTimeOut() < 1 && mode == IDLE) {
#endif
        if (startedUp) mode = ACTIVE;
    }

    // --- Actions and vehicle-specific loop ---
#ifdef NUM_ACTIONS
    for (int n = 0; n < NUM_ACTIONS; n++) myActionList[n].update();
#endif
    platformLoop();   // sequences, background audio, vehicle-specific I/O

    // Timeout nudge (non-CRSF radio only — CRSF uses real-time millis on Core 1)
#ifndef USE_CRSF
    nudgeTimeOut();
#endif

    // --- Brake timer ---
#ifdef USE_MOTOR
    if (brakeTimer > 0) { brakeTimer--; brakeState = 0; }
    if (brakeTimer == 0) brakeState = 1;
#endif

    // --- Volume control ---
#if USE_AUDIO >= 1
    static int volume;
    if (channels[VOLUME_CHANNEL] != volume) {
        volume = channels[VOLUME_CHANNEL];
        player1.setVol(map(volume, 0, 255, 0, 32));
#if USE_AUDIO >= 2
        player2.setVol(map(volume, 0, 255, 0, 32));
#endif
    }
#endif

    watchdog_update();

    // -------------------------------------------------------------------------
    // End of 20Hz loop
    // -------------------------------------------------------------------------

    // Screen update on separate (slower) timer
#ifdef USE_OLED
    static unsigned long screentimer;
    if (millis() > screentimer + 99) {
        screentimer = millis();
        processScreen(mode, position);
    }
#endif

} // end loop()

//////////////////////////////////////////////////////////////////////////////////////////////
// CORE 1 — USB joystick (LUMI) and platform Core 1 tasks
//////////////////////////////////////////////////////////////////////////////////////////////
void setup1() {
    // Wait until Core 0 has finished all hardware initialisation.
    // setup1() starts concurrently with setup() on RP2040 — without this
    // barrier, Core 1 may touch Serial2/I2C/SPI before Core 0 has configured them.
    extern volatile bool _core0SetupDone;
    while (!_core0SetupDone) tight_loop_contents();

#ifdef USB_JOYSTICK
    Joystick.begin();
#endif
#ifdef USE_CRSF
    crsfCore1Setup();   // CRSF reception runs entirely on Core 1
#endif
    platformSetup1();   // vehicle-specific Core 1 init (e.g. Lumi/Scuba audio)
}

void loop1() {
#ifdef USE_CRSF
    crsfCore1Loop();    // runs as fast as possible — no 20Hz gate
#endif

    static unsigned long looptime1;
    if (millis() < looptime1 + 49) return;
    looptime1 = millis();

#ifdef USB_JOYSTICK
    Joystick.X(map(channels[2], 0, 255, 0, 1023));
    Joystick.Y(map(channels[3], 0, 255, 0, 1023));
    Joystick.Z(map(channels[5], 0, 255, 0, 1023));
    Joystick.Zrotate(map(channels[9], 0, 255, 0, 1023));
    if (channels[11] & 1 << 4) Joystick.button(1, true); else Joystick.button(1, false);
    if (channels[11] & 1 << 6) Joystick.button(4, true); else Joystick.button(4, false);
#endif

    platformLoopCore1(); // vehicle-specific Core 1 work (audio etc.)
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_RS485
void ProcessRS485Data(int ID, int dataLength, unsigned char *Data) {}
#endif

#ifndef USE_CRSF
void ProcessRadioData(int ID, int dataLength, unsigned char *Data) {
    if (digitalRead(LED_BUILTIN)) digitalWrite(LED_BUILTIN, LOW);
    else                          digitalWrite(LED_BUILTIN, HIGH);
    if (Data[0] == 0x03) {
        for (int n = 0; n < NUM_CHANNELS; n++) channels[n] = Data[n + 1];
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// OLED screen
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_OLED
void processScreen(int mode, int position) {
    static int menu = 1;
    display.clearDisplay();
#ifdef OLED_ROTATE
    display.setRotation(2);
#endif
    if (menu == 1) {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        // --- Top line ---
        // x=0:  RF link status (7 chars)
        // x=42: mode ACTV/IDLE
        // x=80: animation/sequence status + platform text (right end)
        display.setCursor(0, 0);
#ifdef USE_CRSF
        CrsfStatus cs = crsfGetStatus();
        switch (cs.linkState) {
            case CRSF_LINK_WAITING: display.print(F("RF:WAIT")); break;
            case CRSF_LINK_ACTIVE:  display.print(F("RF:OK  ")); break;
            case CRSF_LINK_LOST:    display.print(F("RF:LST")); break;
        }
#else
        if (mode == ACTIVE) display.print(F("RF:--- "));
        else                display.print(F("RF:--- "));
#endif
        display.setCursor(42, 0);
        if (mode == ACTIVE) display.print(F("ACTV"));
        if (mode == IDLE)   display.print(F("IDLE"));

        display.setCursor(80, 0);
#ifdef ANIMATION_KEY
        if (animation.isPlaying()) {
            display.print(F("seq "));
            display.print(animation.elapsedSeconds());
            display.print(F("s"));
        } else
#endif
#ifdef EXPO_KEY
        if (expanimation.isPlaying())      { display.print(F("expo")); }
        else
#endif
        platformScreen();   // vehicle-specific (e.g. "jaws", "look")

        // --- Bar graph: channels 0-13, y=8..32 ---
        for (int n = 0; n < NUM_CHANNELS - 2; n++)
            display.fillRect(n * 6, 32 - channels[n] / 8, 4, 32, SSD1306_INVERSE);

        // Encoder position bar on right edge
        display.fillRect(124, 0, 4, position, SSD1306_WHITE);

        // --- Bottom-right: keypad indicator (x=92, y=24) ---
#ifdef KEYPAD_CHANNEL
        if (channels[KEYPAD_CHANNEL] > 1) {
            display.setCursor(110, 24);
            display.print((char)(channels[KEYPAD_CHANNEL]));
        }
#endif

    } else if (menu == 2) {
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.println(F("1234567890 actions"));
#ifdef NUM_ACTIONS
        for (int i = 0; i < NUM_ACTIONS; i++) {
            if (myActionList[i].getState() == 1)
                display.fillRect(i * 6, 9, 5, 5, SSD1306_INVERSE);
            else
                display.drawRect(i * 6, 9, 5, 5, SSD1306_INVERSE);
        }
#endif
    }
    display.display();
}
#endif