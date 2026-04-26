#ifndef CRSF_CHANNELS_H
#define CRSF_CHANNELS_H

// ═══════════════════════════════════════════════════════════════════════════════
// PicoRemote / PicoReceiver — shared CRSF channel protocol definition
//
// Include this on BOTH the transmitter (picoRemote) and all receiver vehicles.
// It defines the fixed mapping of logical data onto CRSF channels 1–16.
//
// All values are 0–255 on the wire (mapped to CRSF range 172–1811).
// ═══════════════════════════════════════════════════════════════════════════════

// ── CRSF channel indices (0-based, as used in rcChannels[] / channels[]) ─────

#define CRSF_CH_AXIS_X1       0   // Primary X axis   (nunchuck X, joystick 1X, BetaFPV CH1)
#define CRSF_CH_AXIS_Y1       1   // Primary Y axis   (nunchuck Y, joystick 1Y, BetaFPV CH2)
#define CRSF_CH_AXIS_X2       2   // Secondary X axis (joystick 2X, BetaFPV CH3, or 127)
#define CRSF_CH_AXIS_Y2       3   // Secondary Y axis (joystick 2Y, BetaFPV CH4, or 127)
#define CRSF_CH_ARM           4   // ARM toggle: CRSF_CHANNEL_MIN=disarmed, MAX=armed
#define CRSF_CH_ANALOG1       5   // Analog aux 1 (volume pot, speed, etc.)
#define CRSF_CH_ANALOG2       6   // Analog aux 2
#define CRSF_CH_ANALOG3       7   // Analog aux 3
#define CRSF_CH_NUNCHUCK_BTN  8   // Nunchuck buttons: 0=none 64=Z 128=C 192=both
#define CRSF_CH_KEYPAD_LO     9   // Keypad bitmask bits 0–7
#define CRSF_CH_KEYPAD_HI    10   // Keypad bitmask bits 8–11 (upper 4 bits always 0)
#define CRSF_CH_SW_MUX_0_3  11   // Switch bank: mux ch 0–3  (2 bits each = 8 bits)
#define CRSF_CH_SW_MUX_4_7  12   // Switch bank: mux ch 4–7  (2 bits each = 8 bits)
#define CRSF_CH_SW_MUX_8_11 13   // Switch bank: mux ch 8–11 (2 bits each = 8 bits)
#define CRSF_CH_SW_MUX_12_15 14  // Switch bank: mux ch 12–15(2 bits each = 8 bits)
#define CRSF_CH_SPARE        15   // Reserved for future use

// ── Switch encoding (2 bits per mux channel, fixed bit position) ──────────────
//
// For mux channel N, its 2-bit state occupies bits (2*(N%4)) and (2*(N%4)+1)
// in the appropriate switch bank channel:
//
//   mux ch 0–3  → CRSF_CH_SW_MUX_0_3
//   mux ch 4–7  → CRSF_CH_SW_MUX_4_7
//   mux ch 8–11 → CRSF_CH_SW_MUX_8_11
//   mux ch 12–15→ CRSF_CH_SW_MUX_12_15
//
// 2-bit state values:
//   0b00 (0) = low   (switch to GND, or single-throw OFF)
//   0b01 (1) = mid   (SPDT centre position — voltage divider midpoint)
//   0b10 (2) = high  (switch to 3.3V, or single-throw ON)
//   0b11 (3) = unused (should not occur)
//
// Thresholds (applied to 0–255 mux reading):
//   < 64  → low  → bit pattern 0b01 in the 2-bit slot
//   64–191→ mid  → bit pattern 0b00
//   > 191 → high → bit pattern 0b10
//
// Single-throw (switchChannel=1) only uses the high bit: 0=off, 2=on.

// ── Receiver-side helpers ─────────────────────────────────────────────────────

// Get the raw 2-bit state (0/1/2) of mux channel N from a 4-byte switch array.
// Usage:
//   uint8_t sw[4];
//   sw[0] = channels[CRSF_CH_SW_MUX_0_3];
//   sw[1] = channels[CRSF_CH_SW_MUX_4_7];
//   sw[2] = channels[CRSF_CH_SW_MUX_8_11];
//   sw[3] = channels[CRSF_CH_SW_MUX_12_15];
//   uint8_t state = SWITCH_STATE(sw, 3);  // state of mux channel 3
#define SWITCH_STATE(sw_array, mux_n) \
    (((sw_array)[(mux_n) / 4] >> (((mux_n) % 4) * 2)) & 0x03)

// Convenience: is mux channel N in the HIGH state?
#define SWITCH_HIGH(sw_array, mux_n)  (SWITCH_STATE(sw_array, mux_n) == 2)

// Convenience: is mux channel N in the LOW state?
#define SWITCH_LOW(sw_array, mux_n)   (SWITCH_STATE(sw_array, mux_n) == 0)

// Convenience: is mux channel N in the MID state (SPDT centre)?
#define SWITCH_MID(sw_array, mux_n)   (SWITCH_STATE(sw_array, mux_n) == 1)

// Reconstruct full 12-bit keypad bitmask from two channel values.
// Bit N set = button N is currently pressed.
// Button order matches keypad.cpp scan order:
//   bit 0='1', 1='4', 2='7', 3='*', 4='2', 5='5', 6='8', 7='0',
//   8='3', 9='6', 10='9', 11='#'
#define KEYPAD_BITMASK(ch_lo, ch_hi) \
    ((uint16_t)(ch_lo) | ((uint16_t)((ch_hi) & 0x0F) << 8))

// Is a specific keypad button pressed? (0–11)
#define KEYPAD_PRESSED(ch_lo, ch_hi, btn) \
    ((KEYPAD_BITMASK(ch_lo, ch_hi) >> (btn)) & 1)

// Nunchuck button decode from CRSF_CH_NUNCHUCK_BTN value (0–255)
#define NUNCHUCK_BTN_Z(val)    ((val) >= 64)   // Z button (or just one button)
#define NUNCHUCK_BTN_C(val)    ((val) >= 128)  // C button
#define NUNCHUCK_BTN_BOTH(val) ((val) >= 192)  // both buttons

#endif // CRSF_CHANNELS_H