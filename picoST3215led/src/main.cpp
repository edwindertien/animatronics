#include <Arduino.h>
#include <ST3215_LED.h>
// Pico pins: UART = 0, TX=GP0, RX=GP1, SR = GP2, WS2812=GP4, 37 LEDs, ST3215 ID = 20
ST3215_LED ledRing(0, 0, 1, 2, 4, 37, 20);

void setup() {
  ledRing.begin(false); // enable debug output
}

void loop() {
  ledRing.update();
}
