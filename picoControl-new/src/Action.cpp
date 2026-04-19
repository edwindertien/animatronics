#include "config.h"
#include "Action.h"
#include "PicoRelay.h"
#if USE_AUDIO >= 1
#include "AudioQueue.h"
#endif

extern bool getRemoteSwitch(char button);  // this wil be provided somewhere
//extern SoftwareSerial player1port;
//extern SoftwareSerial player2port;
//extern void writeRelay(int relay,bool state);
extern PicoRelay relay;

// now with function overloading. 

Action::Action(char button, int relaynr, int mode) {
  this->button = button;
  this->relaynr = relaynr;
  this->mode = mode;
  state = 0;
  previousState = 0;
  init();
}

Action::Action(char button, int relaynr, int mode, Motor* motor, int motorvalue) {
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
Action::Action(char button, int relaynr, int mode, Motor* motor, int motorvalue, int track,  DFRobot_DF1201S* player) {
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
    if (getRemoteSwitch(button) && state == 0 && previousState == 0) {
      state = 1;
      trigger(); 
    } else if (!getRemoteSwitch(button) && state == 1 && previousState == 1) {
      state = 0;
      stop();
    }
  } else if (mode == TOGGLE) {
    if (getRemoteSwitch(button) && state == 0 && previousState == 0) {
      state = 1;
      trigger();
    } else if (getRemoteSwitch(button) && state == 1 && previousState == 0) {
      state = 0;
      stop();
    }
  }
  else if (mode == TRIGGER) {
    if (getRemoteSwitch(button)  && previousState == 0){
      trigger();
      
    }
  }
  previousState = getRemoteSwitch(button);
}
void Action::trigger() {
    if (relaynr >= 0) {
        relay.writeRelay(relaynr, HIGH);
    }
    if (motor != nullptr) {
        motor->setSpeed(motorvalue, 0);
    }
#if USE_AUDIO >= 1
    if (player == &player1 && track > 0) {
        audioQueue.enqueue(AUDIO_PLAY, 1, track);
    }
#endif
#if USE_AUDIO >= 2
    else if (player == &player2 && track > 0) {
        audioQueue.enqueue(AUDIO_PLAY, 2, track);
    }
#endif
}

void Action::stop() {
    relay.writeRelay(relaynr, LOW);
    if (motor != nullptr) {
        motor->setSpeed(0, 0);
    }
#if USE_AUDIO >= 1
    if (player == &player1) {
        audioQueue.enqueue(AUDIO_PAUSE, 1);
#ifdef BACKGROUND_TRACK_1
        scheduleBgResume1();
#endif
    }
#endif
#if USE_AUDIO >= 2
    if (player == &player2) {
        audioQueue.enqueue(AUDIO_PAUSE, 2);
#ifdef BACKGROUND_TRACK_2
        scheduleBgResume2();
#endif
    }
#endif
}

int Action::getState() {
  return state;
}