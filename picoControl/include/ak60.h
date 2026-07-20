#pragma once
// AK60 (CubeMars) Servo-mode CAN driver — VESC-style protocol, ported from
// TMotor_ServoConnection.cpp (confirmed working over MCP2515) and adapted
// for ACAN2040 (RP2040 PIO software CAN). Shared across any platform that
// wires up a compatible CAN transceiver — currently platform_experimental.cpp
// and platform_washmachine.cpp.
//
// Wiring assumed: M5Stack CAN unit (CA-IS3050G), GP2=CAN_TX, GP3=CAN_RX,
// 1Mbps, motor CAN ID 104 (set via CubeMars R-Link tool, Servo mode).
// Override AK60_CAN_TX/AK60_CAN_RX/AK60_ID by #define-ing before this
// include if a platform wires it up differently.
//
// NOTE: including this file pulls in <ACAN2040.h>, which is only present
// in environments that list it in platformio.ini lib_deps (currently
// experimental and washmachine). Any platform file that includes ak60.h
// must be excluded from other environments' build_src_filter, and ak60.cpp
// itself must be excluded from environments that don't use it — see
// platformio.ini.

#include <Arduino.h>
#include <ACAN2040.h>

#ifndef AK60_CAN_TX
#define AK60_CAN_TX 2
#endif
#ifndef AK60_CAN_RX
#define AK60_CAN_RX 3
#endif
#ifndef AK60_ID
#define AK60_ID 104
#endif

// Call once per loop tick — idempotent, only actually starts CAN the first
// time it's called (mirrors the original "deferred to first platformLoop()
// tick" pattern, now hidden inside the module).
void ak60Begin();
bool ak60Ready();   // true once ak60Begin() has completed its one-time init

// Servo-mode commands
void ak60SetPosSpd(float posDeg, int16_t spd, int16_t accel);  // speed/accel-limited move
void ak60SetPos(float posDeg);                                  // plain move, motor's default profile
void ak60SetCurrent(float amps);                                // 0A = backdrivable
void ak60SetOrigin(uint8_t mode);  // 0=temporary, 1=permanent (flash-saved), 2=restore factory default

// Feedback — updated from the last successfully-parsed status frame
float   ak60Pos();
float   ak60Vel();
float   ak60Cur();
int8_t  ak60Temp();
uint8_t ak60Err();

// Diagnostics
void ak60PrintStatus();      // one-shot CAN stats + feedback dump
void ak60DebugPrint();       // self-rate-limited (500ms) raw-frame + feedback print — call every loop, no-op unless enabled
void ak60SetDebugStream(bool on);  // enable/disable ak60DebugPrint() output — off by default
bool ak60DebugStream();      // current on/off state
uint32_t ak60TxTotal();      // cumulative successful CAN sends, for rate measurement