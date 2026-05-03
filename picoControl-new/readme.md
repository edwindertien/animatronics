# PicoControl — Animatronic Robot Control Firmware

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

## Architecture

### Dual-core split

| Core 0 (20 Hz loop) | Core 1 (free-running) |
|---|---|
| Read `channels[]` from Core 1 | CRSF receive + parse |
| Update Actions and Sequences | Map 0–255 per channel |
| RS485 / motor / servo output | Link stats (RSSI, LQ) |
| OLED display | Audio (SCUBA, AMI, LUMI) |
| Animation playback | |

Cores share data via RP2040 SDK mutexes (`pico/mutex.h`).

### Channel protocol — `include/crsf_channels.h`

All 16 channels arrive as 0–255 (mapped from CRSF 172–1811).
**This file must be identical on transmitter and receiver.**

```
channels[0]  CRSF_CH_AXIS_X1        Primary X axis (127=centre)
channels[1]  CRSF_CH_AXIS_Y1        Primary Y axis (127=centre)
channels[2]  CRSF_CH_AXIS_X2        Secondary X (127 if unused)
channels[3]  CRSF_CH_AXIS_Y2        Secondary Y (127 if unused)
channels[4]  CRSF_CH_ARM            0=disarmed, 255=armed
channels[5]  CRSF_CH_ANALOG1        Volume/speed pot (0–255)
channels[6]  CRSF_CH_ANALOG2        Spare analog
channels[7]  CRSF_CH_ANALOG3        Spare analog
channels[8]  CRSF_CH_NUNCHUCK_BTN   0=none 64=Z 128=C 192=both
channels[9]  CRSF_CH_KEYPAD_LO      Keypad bits 0–7
channels[10] CRSF_CH_KEYPAD_HI      Keypad bits 8–11
channels[11] CRSF_CH_SW_MUX_0_3     Switch bank A (mux ch 0–3, 2 bits each)
channels[12] CRSF_CH_SW_MUX_4_7     Switch bank B (mux ch 4–7)
channels[13] CRSF_CH_SW_MUX_8_11    Switch bank C (mux ch 8–11)
channels[14] CRSF_CH_SW_MUX_12_15   Switch bank D (mux ch 12–15)
channels[15] CRSF_CH_SPARE          Always 127
```

Switch state per mux channel: `SWITCH_HIGH(sw,n)`, `SWITCH_MID(sw,n)`, `SWITCH_LOW(sw,n)`
Keypad: `KEYPAD_PRESSED(lo, hi, KEY_BIT_5)` etc.
Speed (nunchuck): `NUNCHUCK_C(channels)` = fast, `NUNCHUCK_Z(channels)` = slow.

---

## Action system

```cpp
Action(mux_channel, relaynr, mode)               // switch → relay
Action(mux_channel, relaynr, mode, &motor, val)   // switch → relay + motor
Action(KEY_ACTION(KEY_BIT_5), relaynr, mode, ..., track, &player1) // keypad → audio
Action(-1, relaynr, mode)                         // internal only (no remote button)
```

**Button encoding:**
- `0–15` = mux switch channel (`getRemoteSwitch(n)`)
- `KEY_ACTION(KEY_BIT_x)` = keypad bit 100+ (`getRemoteKey(bit)`)
- `-1` = never fires from remote

**Modes:** `DIRECT` (held), `TOGGLE` (press/press), `TRIGGER` (one-shot)

---

## Animation system

### `animationStep` struct (`include/Animation.h`)

9 bytes per step recorded at 20 Hz. Field names match `crsf_channels.h`:

```cpp
typedef struct {
  uint8_t axis_x;        // CRSF_CH_AXIS_X1
  uint8_t axis_y;        // CRSF_CH_AXIS_Y1
  uint8_t nunchuck;      // CRSF_CH_NUNCHUCK_BTN (0/64/128/192)
  uint8_t keypad_lo;     // CRSF_CH_KEYPAD_LO (bits 0–7)
  uint8_t analog1;       // CRSF_CH_ANALOG1 (volume/speed)
  uint8_t sw_mux_0_3;    // CRSF_CH_SW_MUX_0_3
  uint8_t sw_mux_4_7;    // CRSF_CH_SW_MUX_4_7
  uint8_t sw_mux_8_11;   // CRSF_CH_SW_MUX_8_11
  uint8_t sw_mux_12_15;  // CRSF_CH_SW_MUX_12_15
} animationStep;
```

Binary layout is **identical** to the original struct — all existing `Track-xxx.h` files
work without modification.

### Recording a new animation

1. In `config.h` for your vehicle, uncomment `#define ANIMATION_DEBUG`
2. Flash and open serial monitor
3. Move controls as desired — each 20 Hz tick prints one line:
   `{axis_x,axis_y,nunchuck,keypad_lo,analog1,sw0-3,sw4-7,sw8-11,sw12-15},`
4. Copy output, paste as the array body in `Track-xxx.h`
5. Set `DEFAULT_STEPS` to the number of lines
6. Comment out `ANIMATION_DEBUG`, rebuild

### Animation trigger

`ANIMATION_KEY` in `config.h` is the **mux channel number** of the toggle switch
that starts/stops the animation. Currently mux channel 12 for animaltroniek vehicles.

---

## Per-vehicle config (`include/config.h`)

Each vehicle block defines hardware capability flags and extern declarations.
All behaviour (action lists, sequences) lives in `src/platform_xxx.cpp`.

### Debug flags (all off by default)

| Flag | Effect |
|---|---|
| `ANIMATION_DEBUG` | Record mode: prints channel values at 20 Hz to serial |
| `RS485_DEBUG` | Mirrors RS485 TX buffer to serial every 1 s |
| `DEBUG` | (unused — prefer ANIMATION_DEBUG) |

---

## CRSF link / RSSI

CRSF runs on **Core 1**, completely isolated from Core 0 processing.
`core1_crsf.cpp` parses both `0x16` (RC channels) and `0x14` (link statistics) frames.

Link statistics exposed to Core 0 via `crsfGetLinkStats(CrsfLinkStats& out)`:
- `rssi_ant1` — uplink RSSI in dBm (signed, e.g. -64). More negative = weaker.
- `link_quality` — uplink LQ 0–100 %
- `downlink_rssi` / `downlink_lq` — nanoRX→TX module link

On sustained RF loss (>500 ms): mode → IDLE, `channels[]` reset to `saveValues`.
Serial prints: `CRSF: LOST`, then `CRSF RESTART #N` with exponential backoff,
then `CRSF: ACTIVE (recovered after N restarts)` on recovery.

---

## OLED display (128×32)

Layout mirrors the picoRemote transmitter, with animation track replacing battery:

```
x=0   Vehicle name      e.g. "VIS"
x=20  Mode              "act" or "idle"
x=44  Animation status  "seq:4s" when playing, blank otherwise
x=86  Link quality      "LQ:100%"

y=9   J1 joystick (nunchuck)
      Nunchuck C/Z dots
      J2 joystick (if connected)
      4×4 switch grid
      3×4 keypad grid
      Signal bars + dBm
y=24  Volume bar
```

---

## RS485 passthrough (animaltroniek vehicles)

`BUFFER_PASSTHROUGH 9` — sends 9 bytes every 20 Hz tick to the receiving unit.
Buffer is assembled in old-protocol layout for compatibility with existing receiver firmware:

```
[0] axis_x
[1] axis_y
[2] speed mode (192=fast/C, 128=slow/Z, 0=stop)
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

1. **`platformio.ini`** — add `[env:myvehicle]` with `-DMYVEHICLE` and exclude other platform files
2. **`config.h`** — add `#ifdef MYVEHICLE` block with feature flags and extern declarations
3. **`src/platform_myvehicle.cpp`** — implement action list, sequences, and the 6 platform functions:
   `platformSetup()`, `platformLoop()`, `platformScreen()`, `platformOnIdle()`,
   `platformSetup1()`, `platformLoopCore1()`

---

## Vehicle status (new transmitter protocol)

| Vehicle | Tested | Notes |
|---|---|---|
| animaltroniek_vis | ✓ | Animation, RS485, switches, keypad working |
| animaltroniek_kreeft | compile ✓ | Same hardware as vis |
| animaltroniek_schildpad | compile ✓ | Empty track |
| animal_love | compile ✓ | ANIMATION_KEY needs mux channel verification |
| scuba | compile ✓ | Audio/sequences untested with new TX |
| ami | compile ✓ | Audio/sequences untested with new TX |
| lumi | compile ✓ | Core 1 audio untested with new TX |
| washmachine | compile ✓ | No animation |