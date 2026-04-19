#pragma once
#ifdef USE_CRSF

#include <Arduino.h>
#include <pico/mutex.h>

// Called from setup1() — initialises the mutex and starts CRSF serial
void crsfCore1Setup();

// Called from loop1() — runs the receive loop, maps channels, handles restart
void crsfCore1Loop();

// --- Interface for Core 0 ---

// True once the first valid frame has been received
bool crsfReady();

// Milliseconds since last valid frame (real-time, not loop-tick based)
uint32_t crsfSilenceMs();

// Copy the current mapped channels[] atomically into dst (16 values, 0-255)
void crsfGetChannels(int* dst, int count);

#endif // USE_CRSF
