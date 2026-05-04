#pragma once
// platform.h — interface between main.cpp and per-vehicle platform_xxx.cpp
//
// Every platform_xxx.cpp must implement these four functions.
// main.cpp calls them at the appropriate points and never needs to know
// which vehicle is active.

#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
extern Adafruit_SSD1306 display;
#endif

// Called once in setup(), after all shared hardware (relay, audio, RS485) is initialised.
void platformSetup();

// Called every 20Hz tick, after myActionList[n].update() for all actions.
// Handles vehicle-specific sequences, background audio, etc.
void platformLoop();

// Called from processScreen() in the OLED update section.
// Print a short vehicle-status string to the display at the current cursor position.
void platformScreen();

// Called on timeout (mode → IDLE). Allows vehicles to put RS485 motors/servos
// into a safe/neutral state. Default implementation may be empty.
void platformOnIdle();

// Core 1 hooks — only meaningful for vehicles that use Core 1 (e.g. LUMI audio).
// Provide empty implementations in platform files that don't need them.
void platformSetup1();
void platformLoopCore1();
