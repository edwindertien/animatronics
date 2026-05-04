#pragma once
#ifdef USE_CRSF

#include <Arduino.h>
#include <pico/mutex.h>

// Link state visible to Core 0
#define CRSF_LINK_WAITING  0   // boot, no frame ever received
#define CRSF_LINK_ACTIVE   1   // receiving good frames
#define CRSF_LINK_LOST     2   // was active, now lost

struct CrsfStatus {
    int      linkState;    // CRSF_LINK_xxx
    uint32_t silenceMs;    // ms since last good frame (0 if never received)
    uint32_t frameCount;   // total good frames received
    uint32_t crcErrors;    // total CRC failures
    uint32_t restarts;     // serial restart attempts
};

// Called from setup1() — wait for Core 0 ready, then init serial and mutex
void crsfCore1Setup();

// Called from loop1() — runs the receive/state-machine loop (no 20Hz gate)
void crsfCore1Loop();

// --- Core 0 interface ---
bool        crsfReady();                          // true when LINK_ACTIVE
bool        crsfLost();                           // true when LINK_LOST (was active, now silent)
uint32_t    crsfSilenceMs();                      // ms since last good frame
void        crsfGetChannels(int* dst, int count); // atomic channel copy
CrsfStatus  crsfGetStatus();                      // full diagnostic snapshot

#endif // USE_CRSF
// Link statistics (RSSI, LQ) — populated from 0x14 LINK_STATISTICS frames
struct CrsfLinkStats {
    int8_t  rssi_ant1;       // dBm, negative (e.g. -64). 0 = not yet received.
    int8_t  rssi_ant2;       // dBm, 0 = antenna not present
    uint8_t link_quality;    // 0-100 %
    int8_t  snr;             // dB
    int8_t  downlink_rssi;   // dBm (nanoRX → betafpv TX module)
    uint8_t downlink_lq;     // 0-100 %
};

// Atomic snapshot of link stats for Core 0
void crsfGetLinkStats(CrsfLinkStats& out);