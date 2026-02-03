#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>

#ifndef WIFIREMOTE_TRY_STATIONNUM
#define WIFIREMOTE_TRY_STATIONNUM 1
#endif

class WIFIremote {
public:
  struct Config {
    const char* ssid = "VacuumBot";
    const char* pass = "12345678";      // >= 8 chars for WPA2
    uint8_t channel = 6;
    bool hidden = false;

    // Force AP IP to a fixed value (recommended for stickers)
    IPAddress apIP      = IPAddress(192,168,4,1);
    IPAddress apGateway = IPAddress(192,168,4,1);
    IPAddress apMask    = IPAddress(255,255,255,0);

    // Safety / inactivity
    uint32_t motorIdleMs = 30000;        // 30s: left/right forced to 0
    uint32_t disconnectIdleMs = 120000;  // 2 min: restart AP/server (force reconnect)

    // “In use” window: /api seen recently
    uint32_t inUseWindowMs = 1500;

    // Single-user ownership lease
    uint32_t ownerLeaseMs = 10000;

    // Single-user UI lock
    bool lockControlPage = true;

    uint32_t actuatorMaxOnMs = 10000; // 10s max ON time for any brush/blower
  };

  WIFIremote();

  bool begin(const Config& cfg);
  void loop();

  // Polling API
  bool apActive() const { return _apUp; }
  IPAddress ip() const { return _apIP; }

  int16_t left() const { return _leftCmd; }
  int16_t right() const { return _rightCmd; }

  bool wideBrush() const { return _wide; }
  bool cornerBrush() const { return _corner; }
  bool blower() const { return _blower; }

  bool inUse() const;

  uint8_t stationCount() const;
  bool hasStations() const { return stationCount() > 0; }

  bool hasOwner() const;
  IPAddress ownerIP() const { return _ownerIP; }
  int32_t ownerId() const;      // -1 if none, else packed IP

  void clearDrive();
  void clearAll();
  void releaseOwner();

private:
  // Captive portal helpers
  void startDns();
  void stopDns();
  bool isCaptiveProbePath(const String& path) const;
  void redirectToRoot(WiFiClient& client);

  // HTTP
  void handleClient();
  void serveIndex(WiFiClient& client, const IPAddress& remoteIP);
  void serveWait(WiFiClient& client);
  void serveStatus(WiFiClient& client);
  void handleApi(WiFiClient& client, const String& url, const IPAddress& remoteIP);

  void sendResponse(WiFiClient& client, const char* code, const char* contentType, const String& body);
  void sendResponseProgmem(WiFiClient& client, const char* code, const char* contentType, const uint8_t* data, size_t len);

  // AP + safety
  void applySafety();
  void restartAP();
  void stopAP();

  // Helpers
  static String urlDecode(const String& s);
  static bool getQueryParam(const String& url, const char* key, String& outValue);
  static int16_t clampi16(int v, int16_t lo, int16_t hi);
  static bool parseBool01(const String& v, bool fallback);
  static uint32_t packIP(const IPAddress& ip);

  bool ownerLeaseValid() const;
  bool isOwnerOrFree(const IPAddress& remoteIP) const;

// Auto-off timers for toggles (0 = currently OFF / no timer running)
uint32_t _wideOnSinceMs   = 0;
uint32_t _cornerOnSinceMs = 0;
uint32_t _blowerOnSinceMs = 0;

  Config _cfg;

  WiFiServer* _server = nullptr;
  DNSServer _dns;
  bool _dnsUp = false;

  bool _apUp = false;
  IPAddress _apIP = IPAddress(192,168,4,1);

  // State
  volatile int16_t _leftCmd = 0;
  volatile int16_t _rightCmd = 0;
  volatile bool _wide = false;
  volatile bool _corner = false;
  volatile bool _blower = false;

  // Activity
  uint32_t _lastApiMs = 0;
  uint32_t _lastActivityMs = 0;

  // Ownership
  bool _hasOwner = false;
  IPAddress _ownerIP = IPAddress(0,0,0,0);
  uint32_t _ownerLastSeenMs = 0;
};
