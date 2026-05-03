#ifndef ACTION_H
#define ACTION_H

#include <Arduino.h>
#include "Motor.h"
#include "Audio.h"      // provides DFRobot_DF1201S type + conditional externs

#define DIRECT 0
#define TOGGLE 1
#define TRIGGER 2

// Button encoding — every mux channel has 3 states (0=low, 1=mid, 2=high):
//   MUX_LOW(n)   = 30+n  mux ch n state=0 (switch at low/off end)
//   MUX_MID(n)   = 50+n  mux ch n state=1 (centre position)
//   MUX_HIGH(n)  = 0-15  mux ch n state=2 (switch at high/on end) — same as plain n
//   KEY_ACTION(bit) = 100+bit  keypad bit via getRemoteKey
//   -1 = internal only, never fires from remote
//
// SW(switch_nr, state) — most readable form:
//   SW(1, 0) = switch 1 low,    SW(1, 1) = switch 1 mid,    SW(1, 2) = switch 1 high
#define MUX_LOW(n)      (30 + (n))
#define MUX_MID(n)      (50 + (n))
#define MUX_HIGH(n)     (n)
#define KEY_ACTION(bit) (100 + (bit))
inline int SW(int n, int state) {
    if (state == 0) return MUX_LOW(n);
    if (state == 1) return MUX_MID(n);
    return MUX_HIGH(n);  // state == 2
}

class Action {
  private:
    Motor* motor = nullptr;
    int motorvalue = 0;
    int tracknr = 0;
    int track = 0;
#if USE_AUDIO >= 1
    DFRobot_DF1201S* player = nullptr;
#endif
    int button;
    int relaynr;
    int mode;
    int state;
    int previousState;
    void init();

  public:
    Action(int button, int relaynr, int mode);
    Action(int button, int relaynr, int mode, Motor* motor, int motorvalue);
#if USE_AUDIO >= 1
    Action(int button, int relaynr, int mode, Motor* motor, int motorvalue, int track, DFRobot_DF1201S* player);
#endif
    void update();
    void trigger();
    void stop();
    int getState();
};
#endif