#pragma once
// crsf_channels.h — shared CRSF channel protocol definition.
// MUST be identical on transmitter and receiver sides.
// All channels are 0-255 (mapped from CRSF 172-1811 range).

// ---------------------------------------------------------------------------
// Channel index definitions
// ---------------------------------------------------------------------------
#define CRSF_CH_AXIS_X1         0   // Primary X axis (0-255, 127=centre)
#define CRSF_CH_AXIS_Y1         1   // Primary Y axis (0-255, 127=centre)
#define CRSF_CH_AXIS_X2         2   // Secondary X axis (127 if unused)
#define CRSF_CH_AXIS_Y2         3   // Secondary Y axis (127 if unused)
#define CRSF_CH_ARM             4   // 0=disarmed, 255=armed
#define CRSF_CH_ANALOG1         5   // Volume/speed pot (0-255)
#define CRSF_CH_ANALOG2         6   // Spare analog
#define CRSF_CH_ANALOG3         7   // Spare analog
#define CRSF_CH_NUNCHUCK_BTN    8   // 0=none, 64=C, 128=Z, 192=both
#define CRSF_CH_KEYPAD_LO       9   // Keypad bits 0-7
#define CRSF_CH_KEYPAD_HI       10  // Keypad bits 8-11
#define CRSF_CH_SW_MUX_0_3      11  // Switch bank A: mux channels 0-3
#define CRSF_CH_SW_MUX_4_7      12  // Switch bank B: mux channels 4-7
#define CRSF_CH_SW_MUX_8_11     13  // Switch bank C: mux channels 8-11
#define CRSF_CH_SW_MUX_12_15    14  // Switch bank D: mux channels 12-15
#define CRSF_CH_SPARE           15  // Always mid (127)

// ---------------------------------------------------------------------------
// Switch state decoding
// Each bank channel packs 4 mux channels × 2 bits:
//   bits [1:0] = mux ch N%4 (0=low, 1=mid, 2=high, 3=unused)
//   bits [3:2] = mux ch N%4+1
//   bits [5:4] = mux ch N%4+2
//   bits [7:6] = mux ch N%4+3
// ---------------------------------------------------------------------------
#define SWITCH_STATE(sw, n)  (((sw)[(n)/4] >> (2*((n)%4))) & 0x03)
#define SWITCH_HIGH(sw, n)   (SWITCH_STATE(sw, n) == 2)
#define SWITCH_MID(sw, n)    (SWITCH_STATE(sw, n) == 1)
#define SWITCH_LOW(sw, n)    (SWITCH_STATE(sw, n) == 0)

// Helper: build sw[4] from channels[] in one call
#define SWITCH_BUILD(sw, ch) do { \
    (sw)[0] = (ch)[CRSF_CH_SW_MUX_0_3];  \
    (sw)[1] = (ch)[CRSF_CH_SW_MUX_4_7];  \
    (sw)[2] = (ch)[CRSF_CH_SW_MUX_8_11]; \
    (sw)[3] = (ch)[CRSF_CH_SW_MUX_12_15]; \
} while(0)

// ---------------------------------------------------------------------------
// Keypad decoding
// 12-key bitmask split across two channels:
//   bit 0='1'  bit 1='4'  bit 2='7'  bit 3='*'
//   bit 4='2'  bit 5='5'  bit 6='8'  bit 7='0'
//   bit 8='3'  bit 9='6'  bit 10='9' bit 11='#'
// ---------------------------------------------------------------------------
#define KEYPAD_BITMASK(lo, hi)       ((uint16_t)(lo) | ((uint16_t)((hi) & 0x0F) << 8))
#define KEYPAD_PRESSED(lo, hi, bit)  ((KEYPAD_BITMASK(lo, hi) >> (bit)) & 1)

// Key bit mapping — matches keypad.cpp scan order on transmitter:
// bit 0='1'  bit 1='4'  bit 2='7'  bit 3='*'
// bit 4='2'  bit 5='5'  bit 6='8'  bit 7='0'
// bit 8='3'  bit 9='6'  bit 10='9' bit 11='#'
#define KEY_BIT_1    0
#define KEY_BIT_4    1
#define KEY_BIT_7    2
#define KEY_BIT_STAR 3
#define KEY_BIT_2    4
#define KEY_BIT_5    5
#define KEY_BIT_8    6
#define KEY_BIT_0    7
#define KEY_BIT_3    8
#define KEY_BIT_6    9
#define KEY_BIT_9    10
#define KEY_BIT_HASH 11

// ---------------------------------------------------------------------------
// ARM signal
// ---------------------------------------------------------------------------
#define CRSF_ARMED(ch)   ((ch)[CRSF_CH_ARM] > 128)
#define CRSF_DISARMED(ch) ((ch)[CRSF_CH_ARM] <= 128)

// ---------------------------------------------------------------------------
// Nunchuck buttons: Z=128(2*64), C=64(1*64), both=192(3*64)
// ---------------------------------------------------------------------------
#define NUNCHUCK_Z(ch)    ((ch)[CRSF_CH_NUNCHUCK_BTN] == 128 || (ch)[CRSF_CH_NUNCHUCK_BTN] == 192)
#define NUNCHUCK_C(ch)    ((ch)[CRSF_CH_NUNCHUCK_BTN] == 64  || (ch)[CRSF_CH_NUNCHUCK_BTN] == 192)
#define NUNCHUCK_BOTH(ch) ((ch)[CRSF_CH_NUNCHUCK_BTN] == 192)