# PicoReceiver — Implementation Context

This document is the handoff for implementing the receiver side of the PicoRemote system. It contains everything needed to write the receiver firmware without revisiting the transmitter codebase or the debugging history.

---

## What the receiver already has

The receiver codebase (separate repo, separate chat) already has:

- A working `CRSF.cpp` / `CRSF.h` that receives RC channel packets from the ELRS module and populates a `channels[]` array (0–255 per channel)
- A `getRemoteSwitch(char button)` helper that reads keypad and switch channels — **this needs updating** to the new protocol
- Vehicle-specific `config.h` / `platform_xxx.cpp` structure
- Motor drivers, servo control, audio players, etc. — all untouched by this work

---

## The one file to copy from the transmitter

**`include/crsf_channels.h`** — copy this verbatim into the receiver's `include/` folder. It defines all channel indices and decoding macros. Both sides must always have identical copies.

---

## CRSF channel protocol (what arrives on the receiver)

All 16 channels arrive as values 0–255 (the CRSF 172–1811 range is mapped back to 0–255 on the receiver side by the existing CRSF driver). Channel indices are 0-based.

```
channels[CRSF_CH_AXIS_X1]       // 0  Primary X  (0–255 analog)
channels[CRSF_CH_AXIS_Y1]       // 1  Primary Y
channels[CRSF_CH_AXIS_X2]       // 2  Secondary X (127 if unused)
channels[CRSF_CH_AXIS_Y2]       // 3  Secondary Y
channels[CRSF_CH_ARM]           // 4  0=disarmed, 255=armed
channels[CRSF_CH_ANALOG1]       // 5  Volume/speed pot (0–255)
channels[CRSF_CH_ANALOG2]       // 6  Spare analog
channels[CRSF_CH_ANALOG3]       // 7  Spare analog
channels[CRSF_CH_NUNCHUCK_BTN]  // 8  0=none, 64=Z, 128=C, 192=both
channels[CRSF_CH_KEYPAD_LO]     // 9  Keypad bits 0–7
channels[CRSF_CH_KEYPAD_HI]     // 10 Keypad bits 8–11
channels[CRSF_CH_SW_MUX_0_3]    // 11 Switch bank A
channels[CRSF_CH_SW_MUX_4_7]    // 12 Switch bank B
channels[CRSF_CH_SW_MUX_8_11]   // 13 Switch bank C
channels[CRSF_CH_SW_MUX_12_15]  // 14 Switch bank D
channels[CRSF_CH_SPARE]         // 15 Always mid
```

---

## Reading switches

The four switch bank channels each pack 4 mux channels × 2 bits:

```c
#include "crsf_channels.h"

// Build the switch array once per loop:
uint8_t sw[4] = {
    channels[CRSF_CH_SW_MUX_0_3],
    channels[CRSF_CH_SW_MUX_4_7],
    channels[CRSF_CH_SW_MUX_8_11],
    channels[CRSF_CH_SW_MUX_12_15]
};

// Query individual switches by their original mux channel number (0–15):
uint8_t state = SWITCH_STATE(sw, 3);  // 0=low, 1=mid, 2=high

if (SWITCH_HIGH(sw, 3))  { /* mux ch 3 is high (switch ON) */ }
if (SWITCH_MID(sw, 7))   { /* mux ch 7 is at SPDT centre */ }
if (SWITCH_LOW(sw, 12))  { /* mux ch 12 is low (switch OFF) */ }
```

The bit position for mux channel N is fixed: `bits (2*(N%4))` and `(2*(N%4)+1)` in bank `N/4`.

**Updating `getRemoteSwitch()`:**

Replace the existing function with:

```c
bool getRemoteSwitch(int mux_channel) {
    uint8_t sw[4] = {
        channels[CRSF_CH_SW_MUX_0_3],  channels[CRSF_CH_SW_MUX_4_7],
        channels[CRSF_CH_SW_MUX_8_11], channels[CRSF_CH_SW_MUX_12_15]
    };
    return SWITCH_HIGH(sw, mux_channel);
}

bool getRemoteSwitchMid(int mux_channel) {
    uint8_t sw[4] = {
        channels[CRSF_CH_SW_MUX_0_3],  channels[CRSF_CH_SW_MUX_4_7],
        channels[CRSF_CH_SW_MUX_8_11], channels[CRSF_CH_SW_MUX_12_15]
    };
    return SWITCH_MID(sw, mux_channel);
}
```

The mux channel numbers on the receiver side should match the transmitter's `switchChannel[]` array for that vehicle — the transmitter packs them in order, so the numbering is preserved.

---

## Reading the keypad

The 12-key bitmask is split across two channels:

```c
uint16_t keys = KEYPAD_BITMASK(channels[CRSF_CH_KEYPAD_LO],
                                channels[CRSF_CH_KEYPAD_HI]);

// Check a specific key (0–11):
if (KEYPAD_PRESSED(channels[CRSF_CH_KEYPAD_LO],
                   channels[CRSF_CH_KEYPAD_HI], 5))  // '5' key (bit 5)

// Key bit mapping (matches keypad.cpp scan order on transmitter):
// bit 0='1'  bit 1='4'  bit 2='7'  bit 3='*'
// bit 4='2'  bit 5='5'  bit 6='8'  bit 7='0'
// bit 8='3'  bit 9='6'  bit 10='9' bit 11='#'
```

Multiple simultaneous key presses are fully supported — check whichever bits you need independently.

**Old receiver code** used `channels[KEYPAD_CHANNEL] == 'key_char'`. That only worked for single presses. Replace all such checks with `KEYPAD_PRESSED(...)`.

---

## Nunchuck buttons

```c
uint8_t btn = channels[CRSF_CH_NUNCHUCK_BTN];
// Values: 0=none pressed, 64=Z only, 128=C only, 192=both

bool zPressed   = (btn == 64  || btn == 192);
bool cPressed   = (btn == 128 || btn == 192);
bool bothPressed = (btn == 192);
```

Do not use bitmask operations — use exact value comparisons. The encoding is `chuck.buttons * 64` where chuck.buttons: Z=1, C=2, both=3.

---

## Axes

```c
int x = channels[CRSF_CH_AXIS_X1];  // 0–255, 127=centre
int y = channels[CRSF_CH_AXIS_Y1];  // 0–255, 127=centre

// Convert to signed -128..+127 if needed:
int sx = (int)channels[CRSF_CH_AXIS_X1] - 127;
int sy = (int)channels[CRSF_CH_AXIS_Y1] - 127;

// Map to motor speed range (example):
int speed = map(channels[CRSF_CH_AXIS_Y1], 0, 255, -255, 255);
```

Note: nunchuck Y is **already corrected** on the transmitter side (inverted before sending). The receiver does not need to invert it.

---

## Volume / speed pot

```c
int vol = channels[CRSF_CH_ANALOG1];  // 0–255
```

This is the `ANALOG1_SRC` mux channel from the transmitter config. For SCUBA it's mux ch 8. The pot doubles as the ARM signal source — its value is sent as-is on CH6, and a clean MIN/MAX arm signal derived from it is sent on CH5.

---

## ARM signal

```c
bool armed = (channels[CRSF_CH_ARM] > 128);
```

This is a clean binary signal — the transmitter overrides it with `CRSF_CHANNEL_MIN` or `CRSF_CHANNEL_MAX` regardless of the pot value. Safe to use directly.

---

## RSSI and telemetry on the receiver

The receiver does **not** need to parse CRSF telemetry — the ELRS module sends telemetry back to the transmitter automatically. However, the receiver OLED (if fitted) could show basic link health by reading the LQ channel that the transmitter reflects via telemetry — but this requires a round-trip.

**More practically:** the receiver can read its own link quality from the ELRS module's CRSF stream. The existing `CRSF.cpp` on the receiver side currently only parses `RC_CHANNELS_PACKED (0x16)` frames. To also receive link stats on the receiver side:

1. The ELRS module sends `LINK_STATISTICS (0x14)` frames addressed to `0xC8` on the receiver UART after each received packet
2. The receiver CRSF parser needs to accept `0xC8` as a valid address byte (it currently only looks for `0xEE` as sync)
3. Parse type `0x14` the same way as on the transmitter — see `crsf.h` struct `CrsfLinkStats`

**Key lesson from transmitter development:** The RSSI field `uplink_RSSI_1` is stored as a **signed byte** (two's complement). Cast `p[0]` as `(int8_t)` on store. Do not negate. Values like `0xC0` = -64 dBm, `0xF6` = -10 dBm. Moving further away makes the value more negative (larger unsigned, smaller signed).

**CRC errors during development:** Loopback of our own TX bytes initially caused CRC errors. Fixed by flushing the Serial RX buffer immediately after `Serial.write()` + `Serial.flush()` and resetting the parser state. The receiver side may not need this since it is full-duplex (RX from module, no TX loopback issue).

---

## Receiver OLED display

The receiver display can reuse the exact same helper functions from the transmitter's `main.cpp`:

- `drawJoystick(cx, cy, r, xVal, yVal, invertY)` — circle with crosshair and position dot
- `drawSignalBars(x, y, rssi_dbm)` — 3 bars, fills based on RSSI strength
- `drawBattery(x, y, pct)` — tiny battery symbol with fill
- `drawSwitches(x, y, sw[4])` — 4×4 grid, dot/open/filled per state
- `drawKeypad(x, y, keyBits)` — 3×4 grid, dot/filled per state
- `drawVolBar(x, y, val)` — horizontal bar with "v" label

Copy these functions verbatim. They only depend on the Adafruit SSD1306 library and the standard `channels[]` array. The layout for a 128×32 display:

```
x=  0   21  24    44    65  84             127
y=0 [NAME  ][🔋xx%  ][LQ:xx%             ]
y=9 [J1◎][z][J2◎][sw4×4][kp3×4][▂▄▆ dBm]
y=24                              [ v ▓░░ ]
```

For the receiver, replace the transmitter name with the vehicle name, and the LQ value comes from `crsfLink.link_quality` if you've added telemetry parsing, or simply omit it.

**Display update rate:** call `processScreen()` at 10 Hz (every 100 ms) — same as transmitter. The SSD1306 `display()` call blocks for a few ms and should not be in the fast CRSF polling loop.

---

## Per-vehicle receiver config

The receiver needs a matching `config.h` with the same axis and switch channel mapping as the transmitter. Key defines to add:

```c
// Which CRSF channels carry what (use crsf_channels.h defines)
#define X_AXIS_CH     CRSF_CH_AXIS_X1
#define Y_AXIS_CH     CRSF_CH_AXIS_Y1
#define VOL_CH        CRSF_CH_ANALOG1
#define ARM_CH        CRSF_CH_ARM

// For switch lookup — mux channel numbers match the transmitter's switchChannel[]
// e.g. for SCUBA: mux ch 1,2,3 are SPDT switches
#define SWITCH_FORWARD  1   // mux ch 1 HIGH = forward
#define SWITCH_LEFT     2   // mux ch 2 LOW  = left, HIGH = right
// etc. — vehicle specific
```

---

## What changed from the old protocol

The old transmitter sent:
- `channels[19]` = single ASCII keypad character (only one key at a time)
- `channels[20..23]` = switch bytes, but the bit positions depended on `channelMap[]` (not fixed)
- `channelMap[]` mapped internal slots to CRSF channels in a vehicle-specific order

The new transmitter sends:
- `channels[19]` = keypad low byte (12-bit bitmask), `channels[25]` = keypad high nibble
- Switch bytes use fixed bit positions regardless of vehicle
- No `channelMap[]` — CRSF channels are filled directly by name using `crsf_channels.h` constants
- CH1–CH4 are always axes (BetaFPV compatible)

The receiver `getRemoteSwitch(char button)` function was designed around the old ASCII keypad encoding. It needs to be replaced with the `SWITCH_STATE()` / `KEYPAD_PRESSED()` macros.

---

## Files to copy from transmitter to receiver

| File | Action |
|------|--------|
| `include/crsf_channels.h` | Copy verbatim — shared protocol definition |
| `include/crsf.h` | Reference for `CrsfLinkStats` struct if adding telemetry parsing |
| Display helpers from `src/main.cpp` | Copy the 6 `draw*()` functions if adding OLED |

---

## Debugging tips

- Add `#define DEBUG_MUX` to print all 16 raw mux channel values at 20Hz — useful to verify wiring before trusting the display
- `crsfFramesOk` and `crsfFramesBadCrc` counters are essential for diagnosing framing issues — if `crcErr` climbs fast, check for loopback issues or baudrate mismatch
- The receiver CRSF parser should accept destination address `0xC8` (flight controller), not just `0xEE` (module) — this was the original bug on the transmitter side too