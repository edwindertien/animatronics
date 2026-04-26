# PicoRemote — Universal RC Transmitter

A multi-vehicle RC transmitter built on the Raspberry Pi Pico (RP2040) using the earlephilhower Arduino core. Communicates over ELRS 4.x via CRSF at 420000 baud, with bidirectional telemetry. One codebase covers all vehicles — select the target in `config.h`.

---

## Hardware

### PCB V2.0

| Item | Detail |
|------|--------|
| MCU | Raspberry Pi Pico (RP2040) |
| RF module | BetaFPV MicroRX 2.4 GHz 1W, ELRS 4.0.0 firmware |
| ELRS settings | 333 Hz packet rate, 16ch/2 switch mode, telemetry 1:128 |
| UART | Serial1 — GP0 TX → module, GP1 RX ← module |
| Telemetry | 1 kΩ resistor between GP0 and GP1 (half-duplex loopback) |
| Mux | 16-channel analog multiplexer on A0, address pins GP16–GP19 |
| Display | SSD1306 128×32 OLED on I2C1 (SCL=GP7, SDA=GP6) |
| Nunchuck | Wii Nunchuck on I2C0 |
| Keypad | 12-button matrix, columns GP2/3/11, rows GP12/13/14/15 |
| Battery gauge | MAX17048 on I2C1 |
| Solder jumper | PCB V2.0 — must be closed to bypass TX buffer for telemetry RX |

**Note on Grove I2C connector:** SCL and SDA are swapped relative to the OLED connector.

**Note on servo cable colour:** black=signal, white=GND, red=VCC. Every mux channel has 10k pull-up/pull-down SIL arrays on the PCB.

### SPDT switch wiring (voltage divider)

Each 3-position switch uses a voltage divider to produce three analogue levels on one mux channel:

```
3.3V ── R1 ── mux input ── R2 ── GND
                  │
              SPDT centre
```

The mux reads ~0V (low), ~1.65V (mid), ~3.3V (high). Thresholds in software: `< 64` = low, `64–191` = mid, `> 191` = high.

---

## Software structure

```
include/
  config.h          — vehicle selection and per-vehicle hardware config
  crsf.h            — CRSF protocol, telemetry structs, TX/RX API
  crsf_channels.h   — SHARED channel protocol (include on receiver too)
  muxcontrol.h      — 16-channel mux driver
  nunchuck.h        — Wii Nunchuck I2C driver
  keypad.h          — 12-button keypad driver
  apcrf.h           — Legacy APC220 RF driver (alternative to CRSF)

src/
  main.cpp          — setup/loop, all sampling, CRSF packing, OLED display
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
#define SCUBA          // ← active vehicle
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
| `usedChannel[]` | Which mux channels to actually read (1=read, 0=return 0) |
| `switchChannel[]` | 0=analog, 1=single-throw switch, 2=SPDT 3-position |
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

Defined in `include/crsf_channels.h` — **include this on the receiver too**.

All 16 values are transmitted as 0–255 mapped to CRSF range 172–1811.

| CRSF CH | Index | Content | Notes |
|---------|-------|---------|-------|
| CH1 | 0 | Primary X axis | Nunchuck X, joystick 1X, or BetaFPV CH1 |
| CH2 | 1 | Primary Y axis | Nunchuck Y, joystick 1Y, or BetaFPV CH2 |
| CH3 | 2 | Secondary X | Joystick 2X, BetaFPV CH3, or 127 (mid) |
| CH4 | 3 | Secondary Y | Joystick 2Y, BetaFPV CH4, or 127 |
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

Each switch bank byte holds 4 mux channels × 2 bits. For mux channel N:
- Byte index: `N / 4` (0=bank A, 1=B, 2=C, 3=D)
- Bit offset: `(N % 4) * 2`
- Values: `0b00`=mid, `0b01`=low, `0b10`=high

Receiver decode:
```c
uint8_t sw[4] = {
    channels[CRSF_CH_SW_MUX_0_3],
    channels[CRSF_CH_SW_MUX_4_7],
    channels[CRSF_CH_SW_MUX_8_11],
    channels[CRSF_CH_SW_MUX_12_15]
};
if (SWITCH_HIGH(sw, 3))  { /* mux ch 3 is high */ }
if (SWITCH_MID(sw, 7))   { /* mux ch 7 is at centre */ }
if (SWITCH_LOW(sw, 12))  { /* mux ch 12 is low */ }
```

### Keypad bitmask

12 keys, one bit each. Bit N set = key N currently pressed.

```
Bit: 0  1  2  3  4  5  6  7  8  9  10  11
Key: 1  4  7  *  2  5  8  0  3  6   9   #
```

Reconstruct on receiver:
```c
uint16_t keys = KEYPAD_BITMASK(channels[CRSF_CH_KEYPAD_LO],
                                channels[CRSF_CH_KEYPAD_HI]);
if (KEYPAD_PRESSED(channels[CRSF_CH_KEYPAD_LO],
                   channels[CRSF_CH_KEYPAD_HI], 0))  // '1' key
```

### BetaFPV compatibility

CH1–CH4 always carry the primary analog axes. A standard BetaFPV transmitter (2×XY joysticks on CH1–4) can drive any PicoReceiver vehicle without code changes, as long as the vehicle maps `CRSF_CH_AXIS_X1/Y1/X2/Y2` correctly.

---

## Telemetry (transmitter receives from module)

The ELRS module sends telemetry frames back on the same wire during the inter-packet gap (~3 ms at 333 Hz). A 1 kΩ resistor between GP0 (TX) and GP1 (RX) feeds these back to the RP2040.

After `crsfSendPacket()` the TX FIFO is flushed and any loopback bytes are discarded before the parser runs. Telemetry frames addressed to `0xC8` (flight controller address) are parsed.

### Available telemetry

**LINK_STATISTICS (0x14)** — populated in `crsfLink`:

| Field | Type | Meaning |
|-------|------|---------|
| `rssi_ant1` | `int8_t` | Uplink RSSI dBm, signed negative (e.g. −60). Stored as raw signed byte. |
| `rssi_ant2` | `int8_t` | Second antenna RSSI (0 if single-antenna module) |
| `link_quality` | `uint8_t` | Uplink LQ 0–100 % |
| `snr` | `int8_t` | Uplink SNR dB |
| `active_antenna` | `uint8_t` | 0 or 1 |
| `rf_mode` | `uint8_t` | Packet rate index |
| `tx_power` | `uint8_t` | TX power index |
| `downlink_rssi` | `int8_t` | Ground→air RSSI (often −1/0xFF = not measured) |
| `downlink_lq` | `uint8_t` | Downlink LQ % |

**RSSI note:** The wire value is a signed byte (two's complement). -10 dBm nearby at 1W/30cm, -64 dBm at a few metres are typical values. Do not negate — use `crsfLink.rssi_ant1` directly as dBm.

**BATTERY_SENSOR (0x08)** — populated in `crsfBattery`:

| Field | Type | Meaning |
|-------|------|---------|
| `voltage` | `float` | Pack voltage in Volts |
| `current` | `float` | Current draw in Amps |
| `capacity_used` | `uint32_t` | mAh consumed |
| `percent` | `uint8_t` | Remaining % |

Diagnostic counters (useful for debugging):
- `crsfRxBytes` — total bytes received on Serial1
- `crsfFramesOk` — frames passing CRC
- `crsfFramesBadCrc` — frames failing CRC

---

## OLED display (128×32)

```
x=  0    21  24      44      65   84              127
y=0 [NAME    ][🔋 xx%     ][LQ:xx%              ]
y=9 [J1◎  ][z][J2◎   ][ sw 4×4 ][ kp 3×4 ][ ▂▄▆ dBm]
y=24                                          [ v ▓░░ ]
```

| Zone | Pixels | Content |
|------|--------|---------|
| Top row | x=0, y=0 | Vehicle name (up to 6 chars) |
| Top row | x=38, y=0 | Battery symbol + % |
| Top row | x=84, y=0 | LQ:xx% |
| J1 | cx=10, cy=19, r=9 | Primary axis joystick (Y inverted for nunchuck) |
| C/Z | x=21, y=13/19 | Nunchuck buttons — dot only when pressed |
| J2 | cx=34, cy=19, r=9 | Secondary axis (crosshair only if not connected) |
| Switches | x=44, y=11 | 4×4 grid, dot=low, open square=mid, filled=high |
| Keypad | x=65, y=11 | 3×4 grid, dot=unpressed, filled=pressed |
| Signal bars | x=84, y=9–15 | 3 bars (RSSI) + dBm value |
| Vol bar | x=84, y=24 | Horizontal bar, `ANALOG1_SRC` pot value |

---

## Debug defines

Add to `config.h` or top of `main.cpp`:

| Define | Effect |
|--------|--------|
| `DEBUG` | Serial print of all channel values every 20Hz |
| `DEBUG_MUX` | Serial print of all 16 raw mux reads every 20Hz |

---

## Building

```ini
; platformio.ini
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico
board_build.core = earlephilhower
framework = arduino
lib_deps =
    adafruit/Adafruit SSD1306@^2.5.11
    adafruit/Adafruit GFX Library@^1.11.10
    adafruit/Adafruit MAX1704X@^1.0.3
```

Select vehicle in `config.h`, build, flash via USB.

---

## Adding a new vehicle

1. Add a new `#ifdef MYVEHICLE` block in `config.h` following the pattern of existing vehicles
2. Define all axis sources, `ANALOG1_SRC`, `ARM_MUX_CHANNEL`
3. Fill in `usedChannel[]`, `switchChannel[]`, `invertChannel[]`
4. Add the vehicle name string to the `processScreen()` name block in `main.cpp`
5. Add the corresponding receiver `config.h` on the vehicle side (see `context.md`)



### notes (process these...)

This remote (board and source) have the following features:
 - CRSF or (own) APC220 RF protocol
 - 16 channel analogue mux
 - I2C Oled display
 - (optional) Wii Nunchuck (I2C)
 - (optional) 12-key keypad

 for every model a different channel map is given

in order to reduce data load, for switch channels 'stuffing' is possible, where every switch channel 
(either single or double switch) occupies 2 bits. (so 32 bits for 16 (double) switches)
the keypad is sent as one single character ('0', '1', .. ,'9', '#', '*')

note that the wiring coulour for the 3 axis joystick is counter-intuitive: 

- black: signal
- white: GND
- red: VCC

Every channel is wired with a standard 'servo' cable: 

brown: GND
red: VCC
orange: signal

Note that the ELRS transmitter (starting version 3.4.3) has to be put into 16ch/2 switch mode. This only works with 100 or 333Hz. 

a section for bi-directional communication has been designed, however not implemented yet (channel 15 and 16 give Vbat and RSSI back?)

in order to bypass this ciruit a small solder jumper is necessary on the bottom of the PCB

For every channel pull up and pull down resistors are mounted in 8 x 4 channel SIL array of 4x 10k. 