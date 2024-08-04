#include "Action.h"

bool getRemoteSwitch(char button);  // this wil be provided somewhere
void writeRelay(int relay,bool state);


Action::Action(char button, int relay, int mode, const char* soundfile,  DFRobot_DF1201S* player) {
  this->button = button;
  this->relay = relay;
  this->mode = mode;
  this->soundfile = soundfile;
  this->player = player;
  state = 0;
  previousState = 0;
  init();
}

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
    if (getRemoteSwitch(button) && previousState == 0){
      trigger();
    }
  }
  previousState = getRemoteSwitch(button);
}
void Action::trigger() {
       writeRelay(relay,HIGH);

      if (player && soundfile !="") {
        player->playSpecFile(soundfile);
        Serial.println(soundfile);
      }
}

void Action::stop() {
        writeRelay(relay,LOW);

      if (player) {
        player->pause();
        Serial.println("pause");
      }
}

int Action::getState() {
  return state;
}