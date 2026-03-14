#include "ActionSequence.h"

ActionSequence::ActionSequence(char button, int mode, bool loop)
  : button_(button),
    mode_(mode),
    loop_(loop),
    state_(0),
    previousState_(0),
    playing_(false),
    startTimeMs_(0),
    nextEventIndex_(0),
    eventCount_(0)
{
}

bool ActionSequence::addEvent(uint32_t timestampMs, EventType type, Action* action) {
  if (eventCount_ >= MAX_SEQUENCE_EVENTS) return false;
  events_[eventCount_].timestampMs = timestampMs;
  events_[eventCount_].type = type;
  events_[eventCount_].action = action;
  eventCount_++;

  // Simple insertion sort by timestamp (events are usually added in order anyway)
  for (int i = eventCount_ - 1; i > 0; --i) {
    if (events_[i].timestampMs < events_[i - 1].timestampMs) {
      ActionEvent tmp = events_[i];
      events_[i] = events_[i - 1];
      events_[i - 1] = tmp;
    }
  }
  return true;
}

void ActionSequence::startInternal(uint32_t nowMs) {
  playing_ = true;
  startTimeMs_ = nowMs;
  nextEventIndex_ = 0;
}

void ActionSequence::stopInternal() {
  playing_ = false;
  // Ensure everything in this sequence is stopped
  for (uint8_t i = 0; i < eventCount_; ++i) {
    if (events_[i].action) {
      events_[i].action->stop();
    }
  }
}

void ActionSequence::start() {
  startInternal(millis());
}

void ActionSequence::stop() {
  stopInternal();
}

void ActionSequence::update() {
  uint32_t nowMs = millis();

  // -------- 1) Handle remote button the same way as in Action::update() -----
  bool buttonPressed = getRemoteSwitch(button_);

  if (mode_ == DIRECT) {
    if (buttonPressed && state_ == 0 && previousState_ == 0) {
      state_ = 1;
      startInternal(nowMs);
    } else if (!buttonPressed && state_ == 1 && previousState_ == 1) {
      state_ = 0;
      stopInternal();
    }
  }
  else if (mode_ == TOGGLE) {
    if (buttonPressed && state_ == 0 && previousState_ == 0) {
      state_ = 1;
      startInternal(nowMs);      // toggle ON
    } else if (buttonPressed && state_ == 1 && previousState_ == 0) {
      state_ = 0;
      stopInternal();            // toggle OFF
    }
  }
  else if (mode_ == TRIGGER) {
    if (buttonPressed && previousState_ == 0) {
      // One button press starts the sequence once.
      // Sequence will stop by itself when finished (or loop forever if loop_==true)
      startInternal(nowMs);
    }
  }
  previousState_ = buttonPressed ? 1 : 0;

  // -------- 2) If not playing, nothing more to do ---------------------------
  if (!playing_ || eventCount_ == 0) return;

  // -------- 3) Dispatch events based on elapsed time ------------------------
  uint32_t elapsed = nowMs - startTimeMs_;

  while (nextEventIndex_ < eventCount_ &&
         events_[nextEventIndex_].timestampMs <= elapsed)
  {
    ActionEvent &ev = events_[nextEventIndex_];

    if (ev.action != nullptr) {
      if (ev.type == EVENT_START) {
        ev.action->trigger();
      } else { // EVENT_STOP
        ev.action->stop();
      }
    }

    nextEventIndex_++;
  }

  // -------- 4) End of sequence? Handle one-shot vs loop ---------------------
  if (nextEventIndex_ >= eventCount_) {
    if (loop_) {
      // restart from beginning
      startInternal(nowMs);
    } else {
      playing_ = false;
      // In TRIGGER mode, you may want to reset state_ here or not;
      // for now, we keep button logic separate.
    }
  }
}
