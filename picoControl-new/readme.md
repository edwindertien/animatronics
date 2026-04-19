# Animatronic Robot Control — RP2040

Firmware for controlling animatronic vehicles and props using a Raspberry Pi Pico (RP2040). Built with PlatformIO + the EarlPhilhower Arduino core. Controlled via ELRS/CRSF radio.

---

## Hardware overview

All vehicles share the same base board (V1, V2 or V3 depending on the build) carrying:

- **RP2040** (Raspberry Pi Pico) — dual-core, 133 MHz
- **Relay board** — PCA9635 (BOARD_V2) or Adafruit PWM driver (BOARD_V1) for switching pneumatics, lights, smoke machines etc.
- **ELRS receiver** — connected to Serial2 at 420000 baud (CRSF protocol)
- **RS485 bus** — for external motor drivers and servo boards
- **SSD1306 OLED display** — 128×32, I2C, status display
- **DFRobot DF1201S audio players** — 1 or 2, connected via SoftwareSerial

---

## Building

Select the target vehicle in `platformio.ini` and run:

```bash
pio run -e scuba          # build for SCUBA only
pio run                   # build all vehicles (catches regressions)
pio run -e scuba -t upload # build and upload
```

### Available environments

| Environment | Vehicle | Audio | Notes |
|---|---|---|---|
| `scuba` | SCUBA animatronic car | 1 player | Jaws sequence, bubble background track |
| `ami` | AMI animatronic car | 2 players | Looking sequence, RS485 eyes |
| `lumi` | LUMI house robot | 2 players | Track selection + sample triggers + joystick emulation for eye control |
| `animaltroniek_vis` | Fish animatronic | none | Recorded track animation |
| `animaltroniek_kreeft` | Crab animatronic | none | Recorded track animation |
| `animaltroniek_schildpad` | Turtle animatronic | none | Recorded track animation (empty track) |
| `animal_love` | Three headed animatronic | none | Recorded track + expo animation |
| `washmachine` | Washing machine animatronic | none | M5 servos, trommel motor |
| `stofzuiger` | Vacuum cleaner machine animatronic | none | under construction |

The vehicle identity is set purely via the build flag (e.g. `-DSCUBA`) — no `vehicle_select.h` is needed.

---

## Architecture

### Dual-core split

```
Core 0 (loop)                    Core 1 (loop1)
─────────────────────────────    ──────────────────────────────
20 Hz main loop:                 CRSF reception (runs freely):
  read channels from Core 1        GetCrsfPacket()
  update actions                   map channels 0-255
  run sequences                    update timeout
  RS485 motor/steering             serial restart on loss
  OLED display                     write to shared channels[]
  animation playback             
                                 Audio (50 Hz, gated):
                                   drainAudioQueue()
                                   processBackground() [SCUBA]
                                   processAudio()      [LUMI]
```

CRSF runs on Core 1 without a 20 Hz gate — it reads as fast as the serial data arrives. This isolates it from SoftwareSerial bit-banging on Core 0, which was the primary cause of frame corruption at 420 kbaud.

Audio players also run on Core 1 for the same reason. Play/pause commands from Core 0 (triggered by `Action` objects) travel via a lock-free `AudioQueue` and are executed on Core 1.

Shared data between cores is protected by RP2040 SDK mutexes (`pico/mutex.h`).

### File layout

```
include/
  config.h            — per-vehicle #defines and feature flags (no instances)
  platform.h          — interface contract for platform_xxx.cpp files
  Action.h            — single button/relay/motor/audio action
  ActionSequence.h    — timed sequence of actions
  Audio.h             — player externs, audioInit, processBackground
  AudioQueue.h        — thread-safe play/pause command queue (Core 0→Core 1)
  core1_crsf.h        — Core 1 CRSF interface for Core 0 to read
  Motor.h             — DC motor driver abstraction
  PicoRelay.h         — relay board abstraction (PCA9635 / PWM)
  RS485Reader.h       — RS485 bus communication
  CRSF.h              — CRSF packet parser
  Animation.h         — recorded animation playback
  Track-xxx.h         — recorded animation data per vehicle

src/
  main.cpp            — Core 0 setup/loop, shared hardware init, no vehicle names
  core1_crsf.cpp      — CRSF reception, channel mapping, serial restart (Core 1)
  Audio.cpp           — audioInit, drainAudioQueue, processBackground
  AudioQueue.cpp      — AudioQueue global instance
  Action.cpp          — Action implementation
  ActionSequence.cpp  — ActionSequence implementation
  Animation.cpp       — Animation playback
  platform_scuba.cpp  — SCUBA action list, jaws sequence, audio
  platform_ami.cpp    — AMI action list, looking sequence, eye servos
  platform_lumi.cpp   — LUMI action list, track/sample audio logic
  platform_animaltroniek.cpp  — VIS/KREEFT/SCHILDPAD (shared motor + actions)
  platform_animal_love.cpp    — ANIMAL_LOVE action list and motors
  platform_washmachine.cpp    — WASHMACHINE motors and M5 servos
  platform_desklight.cpp      — DESKLIGHT (stub)
  Motor.cpp / PicoRelay.cpp / CRSF.cpp / RS485Reader.cpp / ...
```

---

## Adding a new vehicle

1. **`platformio.ini`** — add a new `[env:myvehicle]` environment with `-DMYVEHICLE` and exclude all other `platform_xxx.cpp` files.

2. **`include/config.h`** — add a `#ifdef MYVEHICLE` block with the feature flags for your hardware:
   ```cpp
   #ifdef MYVEHICLE
   #define BOARD_V2 (1)
   #define USE_OLED (1)
   #define USE_CRSF (1)
   #define CRSF_CHANNEL_OFFSET 3
   #define NUM_CHANNELS 16
   #define USE_RS485 (1)
   #define RS485_BAUD 57600
   #define SWITCH_CHANNEL 5
   #define KEYPAD_CHANNEL 3
   #define VOLUME_CHANNEL 4
   #define NUM_ACTIONS 4
   extern const int saveValues[];
   extern Action myActionList[NUM_ACTIONS];
   #endif
   ```

3. **`src/platform_myvehicle.cpp`** — implement the platform interface:
   ```cpp
   #ifdef MYVEHICLE
   #include "config.h"
   #include "platform.h"

   const int saveValues[] = { 127, 127, 0, 0, ... };

   Action myActionList[NUM_ACTIONS] = {
       Action(0, 0, DIRECT),    // switch 0 → relay 0
       Action(1, 1, DIRECT),    // switch 1 → relay 1
       // ...
   };

   void platformSetup()     { /* RS485 init, configureSequences(), etc. */ }
   void platformLoop()      { /* sequences, background audio */ }
   void platformScreen()    { /* display.print(F("status")); */ }
   void platformOnIdle()    { /* safe state on RF loss */ }
   void platformSetup1()    { audioInit(); }   // if audio used
   void platformLoopCore1() { drainAudioQueue(); }
   #endif
   ```

That's the complete change set — `main.cpp` and all shared library files remain untouched.

---

## Feature flags reference (`config.h`)

| Flag | Effect |
|---|---|
| `BOARD_V1` | Uses Adafruit PWM driver (PCA9685) for relays |
| `BOARD_V2` | Uses PCA9635 for relays |
| `BOARD_V3` | No relay chip (direct GPIO or external) |
| `USE_AUDIO 0/1/2` | No audio / player1 only / player1+player2 |
| `BACKGROUND_TRACK_1 n` | Track `n` loops continuously on player1; interrupted by foreground sounds and resumed automatically |
| `USE_RS485` | Enables RS485 bus (Serial1 + MAX485) |
| `USE_MOTOR` | Enables DC motor cross-mix drive logic in main loop |
| `USE_SPEEDSCALING` | Three-speed mode (HIGH/LOW/MAX_SPEED via channel 2) |
| `USE_OLED` | Enables SSD1306 128×32 display |
| `USE_CRSF` | ELRS/CRSF radio on Serial2 at 420000 baud |
| `USE_ENCODER` | Quadrature encoder on PIO state machine |
| `USE_M5_SERVOS` | M5Stack 8-channel servo unit (WASHMACHINE) |
| `ANIMATION_KEY n` | Switch bit `n` triggers recorded animation playback |
| `EXPO_KEY n` | GPIO pin `n` (INPUT_PULLUP) triggers expo animation |
| `ANIMATION_TRACK_H "file.h"` | Recorded animation data file to include |
| `BUFFER_PASSTHROUGH n` | RS485 transparent passthrough of first `n` channels |
| `NUM_ACTIONS n` | Number of entries in `myActionList[]` |
| `SWITCH_CHANNEL n` | First channel index for 32-bit switch bitfield |
| `KEYPAD_CHANNEL n` | Channel index carrying keypad character value |
| `VOLUME_CHANNEL n` | Channel index for volume control (0-255 → 0-32) |

---

## Action system

An `Action` maps a remote button or switch to a relay, motor and/or audio track. It is updated every 20 Hz tick.

```cpp
// Relay only
Action(button, relaynr, mode)

// Relay + motor speed
Action(button, relaynr, mode, &motor, motorvalue)

// Relay + motor + audio (requires USE_AUDIO >= 1)
Action(button, relaynr, mode, &motor, motorvalue, track, &player1)
```

**Button values:**
- `'0'`–`'9'`, `'*'`, `'#'` — keypad characters (matched against `channels[KEYPAD_CHANNEL]`)
- `0`–`31` — switch bits spread across `channels[SWITCH_CHANNEL]` through `channels[SWITCH_CHANNEL+3]`

**Modes:**
- `DIRECT` — active while button held; stops on release
- `TOGGLE` — press to start, press again to stop
- `TRIGGER` — fires once on press edge; runs to completion

---

## ActionSequence system

A timed sequence of `EVENT_START` / `EVENT_STOP` events dispatched by elapsed milliseconds. Call `addEvent()` in `configureSequences()` and `update()` in `platformLoop()`.

```cpp
ActionSequence jaws(4, TRIGGER, false);  // button=switch bit 4, TRIGGER, no loop

void configureSequences() {
    jaws.addEvent(0,      EVENT_START, &myActionList[1]);  // audio on
    jaws.addEvent(17500,  EVENT_START, &myActionList[12]); // smoke on
    jaws.addEvent(20000,  EVENT_STOP,  &myActionList[12]); // smoke off
    jaws.addEvent(20000,  EVENT_START, &myActionList[6]);  // hood open
    jaws.addEvent(23500,  EVENT_STOP,  &myActionList[6]);
    jaws.addEvent(23500,  EVENT_START, &myActionList[7]);  // hood close
    jaws.addEvent(24900,  EVENT_STOP,  &myActionList[7]);
    jaws.addEvent(24900,  EVENT_STOP,  &myActionList[1]);  // audio off
}
```

Maximum 16 events per sequence (`MAX_SEQUENCE_EVENTS` in `ActionSequence.h`).

---

## CRSF / ELRS radio

- Receiver connected to Serial2 (TX=pin 8, RX=pin 9), 420000 baud
- Runs entirely on **Core 1** — immune to Core 0 timing jitter and SoftwareSerial interference
- ELRS transmitter settings: packet rate 150 Hz (or 333 Hz), 16 channels, telemetry 1:128
- On sustained RF loss (>500 ms): serial port is restarted automatically
- Mode transitions: `IDLE → ACTIVE` when silence < 50 ms; `ACTIVE → IDLE` when silence > 500 ms
- The animation key channel (`channels[8]`) is **not reset** on RF dropout, so a running animation survives brief disconnections

---

## Board versions

| Version | Relay chip | Notes |
|---|---|---|
| V1 | Adafruit PCA9685 (PWM) | Original board; wire missing between radio VCC and 3.3V |
| V2 | PCA9635 (8-bit LED driver) | Current production board |
| V3 | None / external | DESKLIGHT, WASHMACHINE |

---

## Dependencies (platformio.ini)

```
adafruit/Adafruit SSD1306 @ ^2.5.11
adafruit/Adafruit GFX Library @ ^1.11.10
robtillaart/PCA9635 @ ^0.6.0
robotis-git/Dynamixel2Arduino @ ^0.8.1
```

The EarlPhilhower RP2040 Arduino core provides `pico/mutex.h`, `SoftwareSerial`, `Serial2` and dual-core `setup1()`/`loop1()` support.