#ifndef MUXCONTROL_H
#define MUXCONTROL_H

#include <Arduino.h>

class MuxControl {
  public:
    MuxControl(int* pins, int size);  // Constructor to initialize the pins
    void initMux();                   // Initialize the multiplexer pins
    int checkMux(int channel);        // Check the multiplexer's output based on the channel

  private:
    int* muxpins;    // Pointer to the array of mux pin numbers
    int size;        // Size of the mux pin array
};

#endif