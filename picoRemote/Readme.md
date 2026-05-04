# PicoRemote — Universal RC Transmitter

A multi-vehicle RC transmitter built on the Raspberry Pi Pico (RP2040) using the earlephilhower Arduino core. Communicates over ELRS 4.x via CRSF at 420000 baud, with bidirectional telemetry. One codebase covers all vehicles — select the target in `config.h`.

---

## Hardware

### PCB V2.0

| Item | Detail |
|------|--------|
| MCU | Raspberry Pi Pico (RP2040) |
| RF module | BetaFPV MicroTX 2.4 GHz 1W, ELRS 4.0.0 firmware |
| ELRS settings | 333 Hz packet rate, 16ch/2 switch mode, telemetry 1:128 |
| UART | Serial1 — GP0 TX → module, GP1 RX ← module |
| Telemetry loopback | 1 kΩ resistor between GP0 and GP1 (half-duplex loopback) |
| Mux | 16-channel analog multiplexer on A0, address pins GP16–GP19 |
| Display | SSD1306 128×32 OLED on I2C1 (SCL=GP7, SDA=GP6) |
| Nunchuck | Wii Nunchuck on I2C0 |
| Keypad | 12-button matrix, columns GP2/3/11, rows GP12/13/14/15 |
| Battery gauge | MAX17048 on I2C1 |
| Solder jumper | PCB V2.0 — must be closed to bypass TX buffer for telemetry RX |

### Critical hardware notes

**1 kΩ resistor between GP0 and GP1** — required for telemetry reception. The ELRS module sends telemetry frames back during the inter-packet gap (~3ms at 333Hz). The resistor creates a half-duplex loopback so the RP2040 RX pin sees these frames. Without it, RSSI and LQ are never received and the OLED signal bars always show empty.

**ELRS firmware version** — must be **4.0.0 or later** on both microTX and nanoRX. The 16ch/2 switch mode required for full channel count was introduced in ELRS 3.4.3. Do not downgrade.

**Solder jumper (PCB V2.0)** — a small solder jumper on the bottom of the PCB must be closed to bypass the TX buffer and allow the telemetry loopback to work.

**Grove I2C connector** — SCL and SDA are swapped relative to the OLED connector.

**Servo cable colour** — counter-intuitive wiring on the 3-axis joystick:
- black = signal, white = GND, red = VCC

Standard servo cables on mux channels:
- brown = GND, red = VCC, orange = signal

**Pull-up/pull-down resistors** — every mux channel has 10k pull-up/pull-down resistors in 8×4 channel SIL arrays on the PCB.

### SPDT switch wiring (voltage divider)

Each 3-position switch uses a voltage divider to produce three analogue levels on one mux channel:

```
3.3V ── R1 ── mux input ── R2 ── GND
                  │
              SPDT centre
```

Software thresholds: `< 64` = low, `64–191` = mid, `> 191` = high.
A 2-position toggle produces only low and high. A potentiometer sweeps through all three zones.

---

## Software structure

```
include/
  config.h          — vehicle selection and per-vehicle hardware config
  crsf.h            — CRSF protocol, telemetry structs, TX/RX API
  crsf_channels.h   — SHARED channel protocol (must be identical on receiver)
  muxcontrol.h      — 16-channel mux driver
  nunchuck.h        — Wii Nunchuck I2C driver
  keypad.h          — 12-button keypad driver
  apcrf.h           — Legacy APC220 RF driver (alternative to CRSF)

src/
  main.cpp          — setup/loop, sampling, CRSF packing, OLED display
  crsf.cpp          — CRSF packet TX, telemetry RX parser
  muxcontrol.cpp    — mux channel read
  keypad.cpp        — keypad scan (returns 12-bit bitmask)
  apcrf.cpp         — legacy RF
```

---

## Vehicle selection

Edit `config.h` and enable exactly one vehicle `#define`:

```c
//#define ANIMAL_LOVE
//#define AMI
#define SCUBA
//#define LUMI
//#define ANIMALTRONIEK_VIS
//#define ANIMALTRONIEK_KREEFT
//#define EXPERIMENTAL
```

Each vehicle block defines:

| Define | Meaning |
|--------|---------|
| `AXIS_X1_SRC` | `channels[]` index for primary X axis |
| `AXIS_Y1_SRC` | `channels[]` index for primary Y axis |
| `AXIS_X2_SRC` | Secondary X (or `AXIS_NOT_CONNECTED` = 255) |
| `AXIS_Y2_SRC` | Secondary Y |
| `ANALOG1_SRC` | Volume pot mux channel index |
| `ARM_MUX_CHANNEL` | Mux channel used as ELRS arm signal |
| `usedChannel[]` | Which mux channels to read (1=read, 0=return 0) |
| `switchChannel[]` | 0=analog, 1=single-throw, 2=SPDT 3-position |
| `invertChannel[]` | 1=invert the mux reading (255-val) |

### Internal `channels[]` array (32 entries)

| Index | Content |
|-------|---------|
| 0–15 | Mux analog reads (0–255) |
| 16 | Nunchuck X axis |
| 17 | Nunchuck Y axis |
| 18 | Nunchuck buttons (`chuck.buttons * 64`: Z=64, C=128, both=192) |
| 19 | Keypad bitmask low byte (bits 0–7) |
| 20 | Switch bank A — mux ch 0–3 packed |
| 21 | Switch bank B — mux ch 4–7 packed |
| 22 | Switch bank C — mux ch 8–11 packed |
| 23 | Switch bank D — mux ch 12–15 packed |
| 24 | Encoder position (LUMI only) |
| 25 | Keypad bitmask high nibble (bits 8–11) |
| 26–31 | Spare |

---

## CRSF channel protocol

Defined in `include/crsf_channels.h` — **copy this file verbatim to the receiver**.

All 16 values are transmitted as 0–255 mapped to CRSF range 172–1811.

| CRSF CH | Index | Content | Notes |
|---------|-------|---------|-------|
| CH1 | 0 | Primary X axis | Nunchuck X / joystick 1X / BetaFPV CH1 |
| CH2 | 1 | Primary Y axis | Nunchuck Y / joystick 1Y / BetaFPV CH2 |
| CH3 | 2 | Secondary X | Joystick 2X / BetaFPV CH3 / 127 if unused |
| CH4 | 3 | Secondary Y | Joystick 2Y / BetaFPV CH4 / 127 if unused |
| CH5 | 4 | ARM | MIN=disarmed, MAX=armed. Driven by `ARM_MUX_CHANNEL`. |
| CH6 | 5 | Analog aux 1 | Volume pot (`ANALOG1_SRC`) |
| CH7 | 6 | Analog aux 2 | Spare |
| CH8 | 7 | Analog aux 3 | Spare |
| CH9 | 8 | Nunchuck buttons | 0=none, 64=Z, 128=C, 192=both |
| CH10 | 9 | Keypad bits 0–7 | Low byte of 12-bit bitmask |
| CH11 | 10 | Keypad bits 8–11 | High nibble (upper 4 bits = 0) |
| CH12 | 11 | Switch bank A | Mux ch 0–3, 2 bits each |
| CH13 | 12 | Switch bank B | Mux ch 4–7, 2 bits each |
| CH14 | 13 | Switch bank C | Mux ch 8–11, 2 bits each |
| CH15 | 14 | Switch bank D | Mux ch 12–15, 2 bits each |
| CH16 | 15 | Spare | Always mid |

### Switch bank encoding

For mux channel N: byte index = `N/4`, bit offset = `(N%4)*2`.
Values: `0b00`=mid, `0b01`=low, `0b10`=high.

```c
uint8_t sw[4] = {
    channels[CRSF_CH_SW_MUX_0_3],  channels[CRSF_CH_SW_MUX_4_7],
    channels[CRSF_CH_SW_MUX_8_11], channels[CRSF_CH_SW_MUX_12_15]
};
if (SWITCH_HIGH(sw, 3))  { /* mux ch 3 HIGH */ }
if (SWITCH_MID(sw, 7))   { /* mux ch 7 MID  */ }
if (SWITCH_LOW(sw, 12))  { /* mux ch 12 LOW */ }
```

### Keypad bitmask

12 keys, one bit each. Multiple simultaneous presses fully supported.

```
Bit: 0  1  2  3  4  5  6  7  8  9  10  11
Key: 1  4  7  *  2  5  8  0  3  6   9   #
```

```c
uint16_t keys = KEYPAD_BITMASK(channels[CRSF_CH_KEYPAD_LO], channels[CRSF_CH_KEYPAD_HI]);
if (KEYPAD_PRESSED(channels[CRSF_CH_KEYPAD_LO], channels[CRSF_CH_KEYPAD_HI], 0)) // '1'
```

### BetaFPV compatibility

CH1–CH4 always carry the primary analog axes, so a standard BetaFPV transmitter can drive any picoControl vehicle without code changes.

---

## Telemetry

**LINK_STATISTICS (0x14)** — `crsfLink`:

| Field | Type | Meaning |
|-------|------|---------|
| `rssi_ant1` | `int8_t` | Uplink RSSI dBm, signed (e.g. −60). Do not negate. |
| `rssi_ant2` | `int8_t` | Second antenna RSSI |
| `link_quality` | `uint8_t` | Uplink LQ 0–100% |
| `snr` | `int8_t` | Uplink SNR dB |
| `downlink_rssi` | `int8_t` | Ground→air RSSI |
| `downlink_lq` | `uint8_t` | Downlink LQ % |

**BATTERY_SENSOR (0x08)** — `crsfBattery`:

| Field | Type | Meaning |
|-------|------|---------|
| `voltage` | `float` | Pack voltage (V) |
| `current` | `float` | Current (A) |
| `capacity_used` | `uint32_t` | mAh consumed |
| `percent` | `uint8_t` | Remaining % |

Diagnostic counters: `crsfRxBytes`, `crsfFramesOk`, `crsfFramesBadCrc`.

---

## OLED display (128×32)

```
x=  0    21  24      44      65   84              127
y=0 [NAME    ][🔋 xx%     ][LQ:xx%              ]
y=9 [J1◎  ][z][J2◎   ][ sw 4×4 ][ kp 3×4 ][ ▂▄▆ dBm]
y=24                                          [ v ▓░░ ]
```

| Zone | Content |
|------|---------|
| x=0, y=0 | Vehicle name |
| x=38, y=0 | Battery % |
| x=84, y=0 | LQ:xx% |
| J1 cx=10,cy=19,r=9 | Primary joystick (Y inverted for nunchuck) |
| x=21, y=13/19 | Nunchuck C/Z dots |
| J2 cx=34,cy=19,r=9 | Secondary joystick (crosshair only if unconnected) |
| x=44, y=11 | 4×4 switch grid: dot=low, open=mid, filled=high |
| x=65, y=11 | 3×4 keypad grid: dot=up, filled=pressed |
| x=84, y=9–15 | Signal bars + dBm |
| x=84, y=24 | Volume bar |

---

## Debug defines

| Define | Effect |
|--------|--------|
| `DEBUG` | Serial print all channel values at 20Hz |
| `DEBUG_MUX` | Serial print all 16 raw mux reads at 20Hz |

---

## Building

```ini
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico
board_build.core = earlephilhower
framework = arduino
lib_deps =
    adafruit/Adafruit SSD1306@^2.5.11
    adafruit/Adafruit GFX Library@^1.11.10
    adafruit/Adafruit MAX1704X@^1.0.3
```

---

## Adding a new vehicle

1. Add `#ifdef MYVEHICLE` block in `config.h`
2. Define axis sources, `ANALOG1_SRC`, `ARM_MUX_CHANNEL`, `usedChannel[]`, `switchChannel[]`, `invertChannel[]`
3. Add vehicle name to `processScreen()` in `main.cpp`
4. Add matching `#ifdef MYVEHICLE` block in picoControl's `config.h`