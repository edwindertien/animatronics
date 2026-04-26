// CRSF protocol — bidirectional (TX channels + RX telemetry)
//
// Spec: https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md
// ELRS sends telemetry frames back on the same wire at 420000 baud.
//
// Hardware requirement for half-duplex on a single wire:
//   Place a 1 kΩ resistor between Serial1 TX and Serial1 RX pins.
//   The module drives the bus; the resistor limits current when both
//   TX and RX are asserted simultaneously.
//   (On PCB v2.0 the solder jumper already bypasses the TX buffer —
//   confirm that jumper is closed before enabling RX parsing.)
//
// Frame types we care about:
//   0x16  RC_CHANNELS_PACKED  — we send this (16 ch × 11-bit)
//   0x14  LINK_STATISTICS     — ELRS sends this (RSSI, LQ, SNR, power)
//   0x08  BATTERY_SENSOR      — ELRS sends this (voltage, current, mAh, %)

#ifndef CRSF_H
#define CRSF_H

#include <Arduino.h>

// ── Addresses ────────────────────────────────────────────────────────────────
#define CRSF_ADDR_BROADCAST        0x00
#define CRSF_ADDR_FLIGHT_CTRL      0xC8
#define CRSF_ADDR_REMOTE           0xEA   // us (the remote / handset)
#define CRSF_ADDR_TX_MODULE        0xEE   // the ELRS TX module

// ── Frame types ──────────────────────────────────────────────────────────────
#define CRSF_FRAMETYPE_RC_CHANNELS 0x16
#define CRSF_FRAMETYPE_LINK_STATS  0x14
#define CRSF_FRAMETYPE_BATTERY     0x08

// ── Timing ───────────────────────────────────────────────────────────────────
#define CRSF_BAUDRATE              420000
#define CRSF_TIME_BETWEEN_FRAMES_US 4000  // 250 Hz max; 3000 for 333 Hz

// ── Channel limits (11-bit CRSF units) ───────────────────────────────────────
#define CRSF_CHANNEL_MIN  172
#define CRSF_CHANNEL_MID  991
#define CRSF_CHANNEL_MAX  1811
#define CRSF_MAX_CHANNEL   16

// ── Packet sizes ─────────────────────────────────────────────────────────────
#define CRSF_PACKET_SIZE   26   // 1 addr + 1 len + 1 type + 22 payload + 1 crc
#define CRSF_FRAME_SIZE_MAX 64

// ── Telemetry data (populated by crsfProcessRx) ──────────────────────────────
struct CrsfLinkStats {
    int8_t   rssi_ant1;      // dBm, already negative (e.g. -60). Cast p[0] as int8_t.
    int8_t   rssi_ant2;      // dBm, already negative. 0 = antenna not present.
    uint8_t  link_quality;   // 0-100 %
    int8_t   snr;            // dB, signed
    uint8_t  active_antenna; // 0 or 1
    uint8_t  rf_mode;
    uint8_t  tx_power;       // index into ELRS power table
    int8_t   downlink_rssi;  // dBm, already negative
    uint8_t  downlink_lq;
    int8_t   downlink_snr;
};

struct CrsfBattery {
    float    voltage;        // Volts
    float    current;        // Amps
    uint32_t capacity_used;  // mAh
    uint8_t  percent;        // 0–100 %
};

// ── Public state ─────────────────────────────────────────────────────────────
extern uint8_t       crsfPacket[CRSF_PACKET_SIZE];
extern int           rcChannels[CRSF_MAX_CHANNEL];
extern uint32_t      crsfTime;
extern CrsfLinkStats crsfLink;
extern CrsfBattery   crsfBattery;
extern bool          crsfLinkUpdated;    // true after a new LINK_STATS frame
extern bool          crsfBatteryUpdated; // true after a new BATTERY frame
extern uint32_t      crsfRxBytes;        // total raw bytes received on Serial1
extern uint32_t      crsfFramesOk;       // frames that passed CRC
extern uint32_t      crsfFramesBadCrc;   // frames with wrong CRC

// ── API ──────────────────────────────────────────────────────────────────────
void crsfInit();

// Call from loop() at full speed — timing is managed internally.
void crsfCallback();

// Returns true if a complete telemetry frame was received and decoded.
// Call this every loop iteration; it reads whatever bytes Serial1 has.
bool crsfProcessRx();

// Helpers
void crsfPreparePacket(uint8_t packet[], int channels[]);
void crsfSendPacket();

// Channel enum (matches ELRS AUX channel convention)
enum CrsfChanOrder {
    THROTTLE,
    AILERON,
    ELEVATOR,
    RUDDER,
    AUX1,   // CH5 — ARM switch in ELRS
    AUX2,
    AUX3,
    AUX4,
    AUX5,
    AUX6,
    AUX7,
    AUX8,
    AUX9,
    AUX10,
    AUX11,
    AUX12,
};

#endif // CRSF_H