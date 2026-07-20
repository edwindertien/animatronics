#include "config.h"
#include "CLI.h"
#ifdef USE_CRSF
#include "core1_crsf.h"
#endif

// External references from main.cpp
extern int  channels[];
extern int  mode;
#ifdef USE_MOTOR
extern bool brakeState;
#endif

// ============================================================================
void processCLI() {
    static char    buf[32];
    static uint8_t len       = 0;
    static bool    debugMode = false;

    // ── Read input ────────────────────────────────────────────────────────────
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (len == 0) continue;
            buf[len] = 0;
            String cmd = String(buf);
            cmd.trim();
            cmd.toLowerCase();
            len = 0;

            // ── Commands ─────────────────────────────────────────────────────
            if (cmd == "help" || cmd == "?") {
                Serial.println(F("--- commands ---"));
                Serial.println(F("  status  — vehicle state, channels, RSSI"));
                Serial.println(F("  debug   — toggle 200ms channel dump"));
                Serial.println(F("  ver     — build info"));
#if defined(EXPERIMENTAL) || defined(WASHMACHINE)
                Serial.println(F("  can     — CAN/AK60 status"));
                Serial.println(F("  zero    — AK60: set permanent zero at current position"));
                Serial.println(F("  stream  — toggle AK60 raw/feedback debug stream (off by default)"));
#endif
#ifdef RELAY_TEST
                Serial.println(F("  t       — relay test"));
                Serial.println(F("  r       — relay status"));
#endif
                Serial.println(F("  help/?  — this list"));

            } else if (cmd == "status") {
                Serial.println(F("--- status ---"));
                Serial.print(F("  vehicle: "));
#if defined(SCUBA)
                Serial.println(F("SCUBA"));
#elif defined(AMI)
                Serial.println(F("AMI"));
#elif defined(STOFZUIGER)
                Serial.println(F("STOFZUIGER"));
#elif defined(EXPERIMENTAL)
                Serial.println(F("EXPERIMENTAL"));
#elif defined(WASHMACHINE)
                Serial.println(F("WASHMACHINE"));
#elif defined(DESKLIGHT)
                Serial.println(F("DESKLIGHT"));
#elif defined(ANIMAL_LOVE)
                Serial.println(F("ANIMAL_LOVE"));
#else
                Serial.println(F("UNKNOWN"));
#endif
                Serial.print(F("  mode:    "));
                Serial.println(mode == 1 ? F("ACTIVE") : F("IDLE"));
#ifdef USE_CRSF
                {
                    CrsfLinkStats ls;
                    crsfGetLinkStats(ls);
                    Serial.print(F("  CRSF:    "));
                    Serial.println(crsfReady() ? F("ready") : F("lost"));
                    Serial.print(F("  RSSI:    "));
                    Serial.println((int)ls.rssi_ant1);
                    Serial.print(F("  LQ:      "));
                    Serial.print((int)ls.link_quality);
                    Serial.println(F("%"));
                }
#endif
#ifdef USE_MOTOR
                Serial.print(F("  brake:   "));
                Serial.println(brakeState ? F("on") : F("off"));
#endif
                Serial.print(F("  ch:      "));
                for (int i = 0; i < NUM_CHANNELS; i++) {
                    Serial.print(i); Serial.print(':');
                    Serial.print(channels[i]);
                    if (i < NUM_CHANNELS - 1) Serial.print(' ');
                }
                Serial.println();

            } else if (cmd == "debug") {
                debugMode = !debugMode;
                Serial.print(F("debug ")); Serial.println(debugMode ? F("ON") : F("OFF"));

            } else if (cmd == "ver") {
                Serial.println(F("picoControl"));
                Serial.print(F("  built: ")); Serial.print(F(__DATE__));
                Serial.print(' '); Serial.println(F(__TIME__));

#if defined(EXPERIMENTAL) || defined(WASHMACHINE)
            } else if (cmd == "can") {
                extern void ak60PrintStatus();
                ak60PrintStatus();

            } else if (cmd == "zero") {
                // Sets a PERMANENT zero point at the AK60's current physical
                // position — flash-saved, survives power cycles. Deliberately
                // a distinct, explicit command name (not a generic "origin N"
                // parser) since this changes what every future position
                // command means. Type exactly "zero" to confirm.
                extern void ak60SetOrigin(uint8_t mode);
                ak60SetOrigin(1);
                Serial.println(F("AK60: permanent zero point set at current position"));

            } else if (cmd == "stream") {
                extern void ak60SetDebugStream(bool on);
                extern bool ak60DebugStream();
                ak60SetDebugStream(!ak60DebugStream());
                Serial.print(F("AK60 stream ")); Serial.println(ak60DebugStream() ? F("ON") : F("OFF"));
#endif

#ifdef RELAY_TEST
            } else if (cmd == "t") {
                extern PicoRelay relay;
                Serial.println(F("=== RELAY TEST ==="));
                relay.relayTest();
                Serial.println(F("=== RELAY TEST DONE ==="));
            } else if (cmd == "r") {
                extern PicoRelay relay;
                relay.relayStatus();
#endif

            } else {
                Serial.print(F("unknown: ")); Serial.println(buf);
                Serial.println(F("type 'help'"));
            }

        } else if (len < 31) {
            buf[len++] = c;
        }
    }

    // ── Debug mode — channel dump every 200ms ─────────────────────────────────
    if (debugMode) {
        static unsigned long lastDbg = 0;
        if (millis() - lastDbg > 200) {
            for (int i = 0; i < NUM_CHANNELS; i++) {
                Serial.print(i); Serial.print(':');
                Serial.print(channels[i]);
                if (i < NUM_CHANNELS - 1) Serial.print(' ');
            }
            Serial.println();
            lastDbg = millis();
        }
    }
}