#include "muxcontrol.h"
extern int usedChannel[];
// Constructor to initialize mux pins and size
MuxControl::MuxControl(int* pins, int size) {
  this->muxpins = pins;
  this->size = size;
}

// Initialize the multiplexer pins
void MuxControl::initMux() {
  for (int i = 0; i < size; i++) {
    pinMode(muxpins[i], OUTPUT);
  }
}

// Check the multiplexer's output for the given channel
int MuxControl::checkMux(int channel) {
  for (int i = 0; i < size; i++) {
    if (channel & (1 << i)) {
      digitalWrite(muxpins[i], HIGH);
    } else {
      digitalWrite(muxpins[i], LOW);
    }
  }
  if(usedChannel[channel]){
  int value = analogRead(A0);
  return map(value, 0, 1023, 0, 255);}
  else return 0;
}