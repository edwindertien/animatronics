# picoControl — Animatronic Robot Control Firmware

RP2040-based receiver firmware for controlling animatronic vehicles via ELRS/CRSF radio.
Built with PlatformIO + EarlPhilhower Arduino core.

---

## Building

```bash
pio run -e animaltroniek_vis    # build one vehicle
pio run                         # build all vehicles (catches regressions)
pio run -e animaltroniek_vis -t upload
```

### Environments

| Environment | Vehicle | Audio | Notes |
|---|---|---|---|
| `animaltroniek_vis` | Fish | none | RS485 passthrough, recorded animation |
| `animaltroniek_kreeft` | Crab | none | RS485 passthrough, recorded animation |
| `animaltroniek_schildpad` | Turtle | none | RS485 passthrough (empty track) |
| `animal_love` | Animal Love | none | RS485 passthrough + expo animation |
| `scuba` | SCUBA diver | 1 player | Jaws sequence, bubble background |
| `ami` | AMI animatronic car | 2 players | Looking sequence, RS485 eyes |
| `lumi` | LUMI light robot | 2 players | Track + sample audio |
| `washmachine` | Washing machine | none | M5 servos, trommel motor |

---

## Hardware

### Board Versions

#### V1 — PCA9685 (PWM LED driver)
- I2C address: `0x40`
- 16-channel PWM output used as relay driver
- `BOARD_V1` in `config.h` → `USE_9685` in `PicoRelay.h`
- Output polarity: `setPWM(n, 0, 4095)` = HIGH = relay OFF; `setPWM(n, 0, 0)` = LOW = relay ON
- **Limitation:** on power-up all outputs default to LOW, briefly activating all relays. Cannot be prevented in software — happens before the RP2040 starts. This was the primary motivation for the V2 board.
- Library: Adafruit PWM Servo Driver

#### V2 — PCA9635 (LED driver)
- I2C address: `0x70` (default; depends on A0-A5 pin configuration on the board)
- 16-channel **open-drain** output
- `BOARD_V2` in `config.h` → `USE_9635` in `PicoRelay.h`
- Output polarity: `PCA963X_LEDON` (01) = output sinks LOW = relay ON; `PCA963X_LEDOFF` (00) = floating/HIGH = relay OFF
- **Advantage:** power-on default is high-impedance — relays stay off at startup
- Must call `setLedDriverMode()` on all 16 channels after `begin()`, otherwise subsequent writes are silently ignored
- Library: robtillaart/PCA9635 v0.6

#### EXTRA_RELAY — 8 additional GPIO relays
Enabled by `EXTRA_RELAY` in `config.h`. Relay numbers 16-23 map to GPIO pins:

| Relay | GPIO |
|---|---|
| 16 | 18 |
| 17 | 19 |
| 18 | 20 |
| 19 | 21 |
| 20 | 22 |
| 21 | 26 |
| 22 | 27 |
| 23 | 28 |

All GPIO relays are low-active: `LOW` = ON, `HIGH` = OFF. Initialised HIGH at startup.

#### Board Version Selection — Critical Rule
`BOARD_Vx` **must** be defined before `#include "Action.h"` in `config.h`, because `Action.h` includes `PicoRelay.h` which uses `BOARD_Vx` to select the chip. It is set once in an early `#if defined(...)` block at the top of `config.h`. For SCUBA/AMI (which can use either board), there is a dedicated section:

```cpp
#if defined(SCUBA) || defined(AMI)
#define BOARD_V2 (1)   // V2 board (PCA9635)
//#define BOARD_V1 (1) // V1 board (PCA9685)
#endif
```

**Never** define `BOARD_Vx` inside a vehicle's own `#ifdef` block — duplicate defines cause the wrong chip to be selected.

### Audio Players
- **Player 1:** DFRobot DF1201S — SoftwareSerial GP6 (RX) / GP7 (TX)
- **Player 2:** DFRobot DF1201S — SoftwareSerial GP16 (RX) / GP17 (TX)
- Initialised on Core 0 in `platformSetup()` with a 5-attempt timeout (2.5s max per player)
- Play/pause commands queued via `AudioQueue` (mutex-protected ring buffer), drained on Core 0 in `platformLoop()`

### RS485
- Hardware UART1, direction controlled via `RS485_SR` pin
- `RS485WriteBuf()` includes `delayMicroseconds(500)` after each packet as an inter-packet gap
- `BUFFER_PASSTHROUGH 9` vehicles send a 9-byte legacy-format buffer every 20Hz tick

---

## Architecture

### Dual-core split

| Core 0 (20 Hz gate) | Core 1 (free-running) |
|---|---|
| Read `channels[]` from Core 1 | CRSF receive + parse |
| Update Actions and Sequences | Map 0-255 per channel |
| RS485 / motor / servo output | Link stats (RSSI, LQ) |
| OLED display | (nothing else — ever) |
| Animation playback | |
| Audio drain + background | |

**Core 1 runs CRSF and nothing else.** `platformSetup1()` and `platformLoopCore1()` are empty for all vehicles. Audio SoftwareSerial calls must never run on Core 1 — they are blocking and halt CRSF reception.

Cores share data via RP2040 SDK mutexes (`pico/mutex.h`).

### Startup sequence
1. Core 0 `setup()` initialises all hardware including `audioInit()` (blocking, up to 2.5s per player)
2. Core 0 sets `_core0SetupDone = true` at the very end of `setup()`
3. Core 1 `setup1()` spins on `_core0SetupDone`, then starts `crsfCore1Setup()`
4. Both cores enter their loops

### Channel protocol — `include/crsf_channels.h`

All 16 channels arrive as 0-255 (mapped from CRSF 172-1811).
**This file must be identical on transmitter and receiver.**

```
channels[0]  CRSF_CH_AXIS_X1        Primary X axis (127=centre)
channels[1]  CRSF_CH_AXIS_Y1        Primary Y axis
channels[2]  CRSF_CH_AXIS_X2        Secondary X (127 if unused)
channels[3]  CRSF_CH_AXIS_Y2        Secondary Y
channels[4]  CRSF_CH_ARM            0=disarmed, 255=armed
channels[5]  CRSF_CH_ANALOG1        Volume/speed pot (0-255)
channels[6]  CRSF_CH_ANALOG2        Spare analog
channels[7]  CRSF_CH_ANALOG3        Spare analog
channels[8]  CRSF_CH_NUNCHUCK_BTN   0=none, 64=Z, 128=C, 192=both
channels[9]  CRSF_CH_KEYPAD_LO      Keypad bits 0-7
channels[10] CRSF_CH_KEYPAD_HI      Keypad bits 8-11
channels[11] CRSF_CH_SW_MUX_0_3     Switch bank A (mux ch 0-3, 2 bits each)
channels[12] CRSF_CH_SW_MUX_4_7     Switch bank B (mux ch 4-7)
channels[13] CRSF_CH_SW_MUX_8_11    Switch bank C (mux ch 8-11)
channels[14] CRSF_CH_SW_MUX_12_15   Switch bank D (mux ch 12-15)
channels[15] CRSF_CH_SPARE          Always 127
```

Switch state per mux channel: `SWITCH_HIGH(sw,n)` (state==2), `SWITCH_MID(sw,n)` (state==1), `SWITCH_LOW(sw,n)` (state==0).
Keypad: `KEYPAD_PRESSED(lo, hi, KEY_BIT_5)` etc.
Nunchuck: `NUNCHUCK_C(channels)` = C pressed (128), `NUNCHUCK_Z(channels)` = Z pressed (64), `NUNCHUCK_BOTH` = both (192).

---

## Action system

```cpp
Action(SW(n,2), relaynr, mode)                               // mux ch n HIGH -> relay
Action(SW(n,1), relaynr, mode)                               // mux ch n MID -> relay
Action(SW(n,0), relaynr, mode)                               // mux ch n LOW -> relay
Action(KEY_ACTION(KEY_BIT_5), relaynr, mode, ..., track, &player1)  // keypad -> audio
Action(-1, relaynr, TRIGGER)                                 // internal/sequence-driven only
```

**Button encoding:**

| Value | Meaning |
|---|---|
| `SW(n, 2)` = n (0-15) | Mux channel n, state HIGH |
| `SW(n, 1)` = 50+n | Mux channel n, state MID |
| `SW(n, 0)` = 30+n | Mux channel n, state LOW |
| `KEY_ACTION(KEY_BIT_x)` = 100+bit | Keypad key |
| `-1` | Never fires from remote |

**Modes:** `DIRECT` (held), `TOGGLE` (press/press), `TRIGGER` (one-shot, no auto-stop from `update()`)

**Important:** actions driven by sequences must use `button=-1` and `TRIGGER` mode. A `DIRECT` action triggered by a sequence is immediately stopped by `update()` on the next tick because the button is not physically held.

---

## Animation system

### `animationStep` struct (`include/Animation.h`)

9 bytes per step recorded at 20 Hz:

```cpp
typedef struct {
  uint8_t axis_x;       // CRSF_CH_AXIS_X1
  uint8_t axis_y;       // CRSF_CH_AXIS_Y1
  uint8_t nunchuck;     // CRSF_CH_NUNCHUCK_BTN (0/64/128/192)
  uint8_t keypad_lo;    // CRSF_CH_KEYPAD_LO (bits 0-7)
  uint8_t analog1;      // CRSF_CH_ANALOG1 (volume/speed)
  uint8_t sw_mux_0_3;   // CRSF_CH_SW_MUX_0_3
  uint8_t sw_mux_4_7;   // CRSF_CH_SW_MUX_4_7
  uint8_t sw_mux_8_11;  // CRSF_CH_SW_MUX_8_11
  uint8_t sw_mux_12_15; // CRSF_CH_SW_MUX_12_15
} animationStep;
```

Binary layout is identical to the original struct — all existing `Track-xxx.h` files work without modification.

### Embedding actuator commands in a track
Switch bank fields encode 4 mux channels x 2 bits each. To activate mux channel N during playback, add the appropriate value to the relevant field. Example: mux ch 2 MID (`SW(2,1)`) in `sw_mux_0_3` (field 5) = add `16`. This allows actuators (sirens, motors, etc.) to be embedded in recorded tracks without a separate sequence.

### Recording a new animation
1. Uncomment `#define ANIMATION_DEBUG` in `config.h` for your vehicle
2. Flash and open serial monitor
3. Move controls — each 20 Hz tick prints one line:
   `{axis_x,axis_y,nunchuck,keypad_lo,analog1,sw0-3,sw4-7,sw8-11,sw12-15},`
4. Copy output, paste as the array body in `Track-xxx.h`
5. Set `DEFAULT_STEPS` to the number of lines
6. Comment out `ANIMATION_DEBUG`, rebuild

### Animation trigger
`ANIMATION_KEY` in `config.h` is the mux channel number. `ANIMATION_KEY_STATE` (0/1/2, default 2) sets which switch state triggers playback:

```cpp
#define ANIMATION_KEY        12   // mux channel
#define ANIMATION_KEY_STATE   2   // HIGH = SW(12,2)
```

The `ANIMATION_KEY_PRESSED` macro in `main.cpp` resolves to the correct `getRemoteSwitch*()` call. The animation key channel is always updated from live CRSF data during playback so releasing the switch correctly stops the animation.

---

## Debug flags

All flags are commented out by default and compiled away when inactive:

| Flag | Effect |
|---|---|
| `ANIMATION_DEBUG` | Prints channel values at 20 Hz for animation recording |
| `ANIMATION_KEY_DEBUG` | Dumps switch banks and animation key state every 1s |
| `RS485_DEBUG` | Mirrors RS485 TX buffer to serial every 1s |
| `RELAY_DEBUG` | Prints `RELAY ON/OFF: n` on every relay state change |
| `RELAY_TEST` | Enables `t` (flash all relays) and `r` (I2C scan) serial commands |
| `AUDIO_DEBUG` | Prints `AUDIO P1/P2 PLAY/PAUSE track=n` |
| `INPUT_DEBUG` | Prints all switch states, keypad bits, and nunchuck value every 500ms |

---

## Per-vehicle config (`include/config.h`)

Key flags:

| Flag | Effect |
|---|---|
| `USE_AUDIO 1/2` | Enable 1 or 2 DFRobot audio players |
| `USE_MOTOR` | Enable DC motor control |
| `USE_RS485` | Enable RS485 output |
| `BUFFER_PASSTHROUGH 9` | Send 9-byte legacy RS485 buffer each tick |
| `USE_OLED` | Enable 128x32 OLED display |
| `USE_CRSF` | Enable ELRS/CRSF radio (Core 1) |
| `USE_SPEEDSCALING` | Enable nunchuck-based speed scaling for motors |
| `ANIMATION_KEY n` | Mux channel number for animation trigger |
| `ANIMATION_KEY_STATE n` | Switch state (0/1/2) that triggers animation |
| `ANIMATION_TRACK_H` | Track file to include (e.g. `"Track-vis.h"`) |
| `BACKGROUND_TRACK_1` | Track number for auto-restarting background audio on player 1 |
| `EXTRA_RELAY` | Enable 8 additional GPIO relay channels (16-23) |
| `EXPO_KEY` | GPIO pin for expo animation trigger (INPUT_PULLUP) |

---

## CRSF link / RSSI

CRSF runs on Core 1, completely isolated from Core 0.
`core1_crsf.cpp` parses both `0x16` (RC channels) and `0x14` (link statistics) frames.

Link statistics via `crsfGetLinkStats(CrsfLinkStats& out)`:
- `rssi_ant1` — uplink RSSI in dBm (signed, e.g. -64)
- `link_quality` — uplink LQ 0-100%
- `downlink_rssi` / `downlink_lq` — nanoRX to TX module link

On RF loss (>500ms): mode to IDLE, `channels[]` reset to `saveValues`.
Serial: `CRSF: LOST` > `CRSF RESTART #N` (exponential backoff) > `CRSF: ACTIVE (recovered)`.

---

## OLED display (128x32)

```
x=0   Vehicle name (3 chars)   SCU / AMI / LUM / KRE / VIS / SCH / LOV / WAS
x=20  Mode                      "act" or "idle"
x=44  Sequence/platform status  "sq:4s" when animation playing, platformScreen() otherwise
x=86  Link quality              "LQ:100%"

Middle: J1 joystick | nunchuck C/Z dots | J2 joystick | 4x4 switch grid | 3x4 keypad | signal bars + dBm
Bottom: Volume bar
```

---

## RS485 passthrough (animaltroniek / animal_love vehicles)

`BUFFER_PASSTHROUGH 9` — 9 bytes sent every 20Hz:

```
[0] axis_x
[1] axis_y
[2] nunchuck raw (0=none, 64=Z, 128=C, 192=both)
[3] keypad ASCII char (first pressed key, 0=none)
[4] volume (analog1)
[5] sw_mux_0_3
[6] sw_mux_4_7
[7] sw_mux_8_11
[8] sw_mux_12_15
```

During animation playback, values come from the track file rather than live RF.

---

## Adding a new vehicle

1. **`platformio.ini`** — add `[env:myvehicle]` with `-DMYVEHICLE`
2. **`config.h`** — add `#ifdef MYVEHICLE` block. Board version in early block only.
3. **`src/platform_myvehicle.cpp`** — implement the 6 platform functions:

```cpp
void platformSetup();       // Core 0: init, audioInit(), configureSequences()
void platformLoop();        // Core 0: sequences, drainAudioQueue(), RS485
void platformScreen();      // Core 0: text for x=44 on OLED top row
void platformOnIdle();      // Core 0: called when RF link lost
void platformSetup1();      // Core 1: leave empty
void platformLoopCore1();   // Core 1: leave empty
```

---

## Vehicle status

| Vehicle | Status | Notes |
|---|---|---|
| animaltroniek_vis | working | Animation, RS485, switches, nunchuck confirmed |
| animaltroniek_kreeft | compile | Same hardware as vis |
| animaltroniek_schildpad | compile | Empty track |
| animal_love | working | Animation, expo sequence, siren, motors confirmed |
| scuba | working | Jaws sequence, audio, relays, snorkel confirmed |
| ami | working | Looking sequence, audio, 24 relays, RS485 eyes confirmed |
| lumi | compile | Untested with new transmitter |
| washmachine | compile | Untested with new transmitter |