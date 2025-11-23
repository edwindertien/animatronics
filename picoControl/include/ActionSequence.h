#pragma once

#include <Arduino.h>
#include "Action.h"

// You probably already have these in Action.h:
extern bool getRemoteSwitch(char button);

// How many events max per sequence:
#define MAX_SEQUENCE_EVENTS 16

enum EventType {
  EVENT_START,
  EVENT_STOP
};

struct ActionEvent {
  uint32_t timestampMs;   // from sequence start
  EventType type;         // start or stop
  Action* action;         // which Action to control
};

class ActionSequence {
public:
  // button/mode just like Action, loop == true makes it continuous
  ActionSequence(char button, int mode, bool loop = false);

  // Configure events (call this in setup())
  bool addEvent(uint32_t timestampMs, EventType type, Action* action);

  // Call this in loop()
  void update();

  // Optional control from code (not just from button)
  void start();
  void stop();

  void setLoop(bool loop) { loop_ = loop; }
  bool isPlaying() const { return playing_; }

private:
  void startInternal(uint32_t nowMs);
  void stopInternal();

  char button_;
  int mode_;            // DIRECT / TOGGLE / TRIGGER
  bool loop_;

  // button state (copied from your Action logic)
  int state_;           // 0 or 1
  int previousState_;   // 0 or 1

  // sequence timing
  bool playing_;
  uint32_t startTimeMs_;
  uint8_t nextEventIndex_;

  // events
  ActionEvent events_[MAX_SEQUENCE_EVENTS];
  uint8_t eventCount_;
};
