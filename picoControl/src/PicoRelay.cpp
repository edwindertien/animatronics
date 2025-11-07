#include "PicoRelay.h"
#include <math.h>
#include <config.h>

PicoRelay::PicoRelay()
#if defined(USE_9685)
    : pwm()
#elif defined(USE_9635)
    : pwm(0x70)
#endif
{
#if defined(LUMI)
    joystickActive = false;
#endif
}

void PicoRelay::begin() {
#if defined(USE_9685)
    //pwm.begin();
    //pwm.setPWMFreq(1000);
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(16000);
  for (int i=0; i<16; i++){
    writeRelay(i,0);
  }
#elif defined(USE_9635)
    pwm.begin();
#endif
}

void PicoRelay::writeRelay(int relaynr, bool state) {
    if (relaynr < 0 || relaynr >= 16) return;

#if defined(USE_9685)
    pwm.setPWM(relaynr, 0, state ? 0 : 4095);
#elif defined(USE_9635)
    pwm.setLedDriverMode(relaynr, state ? PCA963X_LEDON : PCA963X_LEDOFF);
#endif
}

#if defined(LUMI)
const uint8_t PicoRelay::driveRelays[12] = {
  0b00001000, 0b00011000, 0b00010000, 0b00110000,
  0b00100000, 0b00100001, 0b00000001, 0b00000011,
  0b00000010, 0b00000110, 0b00000100, 0b00001100
};

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
        int relayNumber = constrain((180 + 360.0 * (atan2(dx, dy) / (2 * PI))) / 30, 0, 11);
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