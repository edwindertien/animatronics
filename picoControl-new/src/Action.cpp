#include "config.h"
#include "Action.h"
#include "PicoRelay.h"
#if USE_AUDIO >= 1
#include "AudioQueue.h"
#endif

extern bool getRemoteSwitch(int mux_channel);
extern bool getRemoteKey(int bit);
extern bool getRemoteSwitchMid(int mux_channel);
extern bool getRemoteSwitchLow(int mux_channel);
extern PicoRelay relay;

static bool _buttonPressed(int button) {
    if (button < 0)    return false;                          // -1 = internal only
    if (button >= 100) return getRemoteKey(button - 100);    // KEY_ACTION(bit)
    if (button >= 50)  return getRemoteSwitchMid(button-50); // MUX_MID(n) / SW(n,1)
    if (button >= 30)  return getRemoteSwitchLow(button-30); // MUX_LOW(n) / SW(n,0)
    return getRemoteSwitch(button);                           // MUX_HIGH(n) / SW(n,2)
}

// now with function overloading. 

Action::Action(int button, int relaynr, int mode) {
  this->button = button;
  this->relaynr = relaynr;
  this->mode = mode;
  state = 0;
  previousState = 0;
  init();
}

Action::Action(int button, int relaynr, int mode, Motor* motor, int motorvalue) {
  this->button = button;
  this->relaynr = relaynr;
  this->mode = mode;
  this->motor = motor;
  this->motorvalue = motorvalue;
  state = 0;
  previousState = 0;
  init();
}

#if USE_AUDIO >= 1
Action::Action(int button, int relaynr, int mode, Motor* motor, int motorvalue, int track,  DFRobot_DF1201S* player) {
  this->button = button;
  this->relaynr = relaynr;
  this->mode = mode;
  this->motor = motor;
  this->motorvalue = motorvalue;
  this->track = track;
  this->player = player;
  state = 0;
  previousState = 0;
  init();
}
#endif

void Action::init() {
  //for (int n = 0; n < 16; n++) pwm.setPWM(n, 0, 4095);
}

void Action::update() {
  if (mode == DIRECT) {
    if (_buttonPressed(button) && state == 0 && previousState == 0) {
      state = 1;
      trigger(); 
    } else if (!_buttonPressed(button) && state == 1 && previousState == 1) {
      state = 0;
      stop();
    }
  } else if (mode == TOGGLE) {
    if (_buttonPressed(button) && state == 0 && previousState == 0) {
      state = 1;
      trigger();
    } else if (_buttonPressed(button) && state == 1 && previousState == 0) {
      state = 0;
      stop();
    }
  }
  else if (mode == TRIGGER) {
    if (_buttonPressed(button)  && previousState == 0){
      trigger();
      
    }
  }
  previousState = _buttonPressed(button);
}
void Action::trigger() {
    if (relaynr >= 0) {
        relay.writeRelay(relaynr, HIGH);
#ifdef RELAY_DEBUG
        Serial.print("RELAY ON: "); Serial.println(relaynr);
#endif
    }
    if (motor != nullptr) {
        motor->setSpeed(motorvalue, 0);
    }
#if USE_AUDIO >= 1
    if (player == &player1 && track > 0) {
        audioQueue.enqueue(AUDIO_PLAY, 1, track);
#ifdef AUDIO_DEBUG
        Serial.print("AUDIO P1 PLAY track="); Serial.println(track);
#endif
    }
#endif
#if USE_AUDIO >= 2
    else if (player == &player2 && track > 0) {
        audioQueue.enqueue(AUDIO_PLAY, 2, track);
#ifdef AUDIO_DEBUG
        Serial.print("AUDIO P2 PLAY track="); Serial.println(track);
#endif
    }
#endif
}

void Action::stop() {
    relay.writeRelay(relaynr, LOW);
#ifdef RELAY_DEBUG
    if (relaynr >= 0) { Serial.print("RELAY OFF: "); Serial.println(relaynr); }
#endif
    if (motor != nullptr) {
        motor->setSpeed(0, 0);
    }
#if USE_AUDIO >= 1
    if (player == &player1) {
        audioQueue.enqueue(AUDIO_PAUSE, 1);
#ifdef AUDIO_DEBUG
        Serial.println("AUDIO P1 PAUSE");
#endif
#ifdef BACKGROUND_TRACK_1
        scheduleBgResume1();
#endif
    }
#endif
#if USE_AUDIO >= 2
    if (player == &player2) {
        audioQueue.enqueue(AUDIO_PAUSE, 2);
#ifdef AUDIO_DEBUG
        Serial.println("AUDIO P2 PAUSE");
#endif
#ifdef BACKGROUND_TRACK_2
        scheduleBgResume2();
#endif
    }
#endif
}

int Action::getState() {
  return state;
}