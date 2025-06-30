#include "PicoRelay.h"

#ifdef USE_9685
PicoRelay::PicoRelay() : pwm() {}
#else
PicoRelay::PicoRelay() : pwm(0x70) {}
#endif

void PicoRelay::begin() {
  pwm.begin();

#ifdef USE_9685
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(16000);
#endif

  // Clear all relays
  for (int n = 0; n < 16; n++) {
    writeRelay(n, LOW);
  }
}



void PicoRelay::writeRelay(int relay, bool state) {
  if (relay < 0 || relay >= 16) return;

#ifdef USE_9685
  pwm.setPWM(relay, 0, state ? 0 : 4095);
#else
  pwm.setLedDriverMode(relay, state ? PCA963X_LEDON : PCA963X_LEDOFF);
#endif
}

#ifdef LUMI
void PicoRelay::joystickToRelays(int x, int y) {
  const int center = 127;
  const int enterThreshold = 60;
  const int exitThreshold = 40;

  int dx = x - center;
  int dy = y - center;
  int distance = sqrt(dx * dx + dy * dy);

  if (!joystickActive && distance > enterThreshold) {
    joystickActive = true;
  } else if (joystickActive && distance < exitThreshold) {
    joystickActive = false;
  }

  if (joystickActive) {
    int relayNumber = constrain((180 + 360.0 * (atan2(dx, dy) / 6.28)) / 30, 0, 11);
    for (int i = 0; i < 6; i++) {
      writeRelay(i, driveRelays[relayNumber] & (1 << i));
    }
  } else {
    for (int i = 0; i < 6; i++) {
      writeRelay(i, LOW);
    }
  }
}
#endif
