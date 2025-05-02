#include "Action.h"

extern bool getRemoteSwitch(char button);  // this wil be provided somewhere
extern void writeRelay(int relay,bool state);

// now with function overloading. 

Action::Action(char button, int relay, int mode) {
  this->button = button;
  this->relay = relay;
  this->mode = mode;
  this->relay1 = -1;
  this->relay2 = -1;
  this->relay3 = -1;
  state = 0;
  previousState = 0;
  init();
}

Action::Action(char button, int relay, int mode, int relay1) {
  this->button = button;
  this->relay = relay;
  this->mode = mode;
  this->relay1 = relay1;
  this->relay2 = -1;
  this->relay3 = -1;
  state = 0;
  previousState = 0;
  init();
}

Action::Action(char button, int relay, int mode, int relay1, int relay2) {
  this->button = button;
  this->relay = relay;
  this->mode = mode;
  this->relay1 = relay1;
  this->relay2 = relay2;
  this->relay3 = -1;
  state = 0;
  previousState = 0;
  init();
}
Action::Action(char button, int relay, int mode, int relay1, int relay2, int relay3) {
  this->button = button;
  this->relay = relay;
  this->mode = mode;
  this->relay1 = relay1;
  this->relay2 = relay2;
  this->relay3 = relay3;
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
  else { Serial.println("error: mode undefined");}
  //previousState = getRemoteSwitch(button);
  previousState = state;
}
void Action::trigger() {
        // Trigger relay action
    if (relay >= 0) {
      writeRelay(relay, HIGH);  // Trigger relay
  }
  if (relay1 >= 0) {
    writeRelay(relay1, HIGH);  // Trigger relay
}
if (relay2 >= 0) {
  writeRelay(relay2, HIGH);  // Trigger relay
}
if (relay3 >= 0) {
  writeRelay(relay3, HIGH);  // Trigger relay
}
  // // Trigger motor speed if motor is initialized
  // if (motor != nullptr) {
  //     motor->setSpeed(motorvalue);  // Example: Setting motor speed to full (255). Adjust as needed.
  //     //Serial.println("Motor speed set.");
  // }

  // // Play sound if a soundfile is provided
  // if (player != nullptr && soundfile != nullptr && *soundfile != '\0') {
  //     player->playSpecFile(soundfile);
  //     //Serial.println(soundfile);
  // }
      //  writeRelay(relay,HIGH);

      // if (player && soundfile !="") {
      //   player->playSpecFile(soundfile);
      //   Serial.println(soundfile);
      // }
}

void Action::stop() {
      // Reset relay
      if (relay >= 0) {writeRelay(relay, LOW);}
      if (relay1 >= 0) {writeRelay(relay1, LOW);}
      if (relay2 >= 0) {writeRelay(relay2, LOW);}
      if (relay3 >= 0) {writeRelay(relay3, LOW);}
      // Pause motor if it's initialized
      // if (motor != nullptr) {
      //     motor->setSpeed(0);  // Stop the motor
      //     Serial.println("Motor stopped.");
      // }
  
      // // Pause sound if player is initialized
      // if (player != nullptr) {
      //     player->pause();
      //     Serial.println("Sound paused.");
      // }
      //   writeRelay(relay,LOW);

      // if (player) {
      //   player->pause();
      //   Serial.println("pause");
      // }
}

int Action::getState() {
  return state;
}