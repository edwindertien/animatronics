#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class ST3215_LED {
public:
    ST3215_LED(uint8_t uartId,
               uint8_t txPin,
               uint8_t rxPin,
               uint8_t srPin,
               uint8_t ledPin,
               uint16_t ledCount,
               uint8_t id);

    void begin(bool debug = true);
    void update();  // call in loop()
    void setID(uint8_t newID);
    uint8_t getID() const { return myID; }

private:
    // UART + LED
    uint8_t uartId;
    uint8_t txPin, rxPin, srPin;
    uint8_t ledPin;
    uint16_t ledCount;
    Adafruit_NeoPixel strip;

    // State
    uint8_t myID;
    uint16_t goalPos;
    uint16_t pattern;
    uint16_t colour;
    uint16_t brightness;

    // Parser state
    enum State { WAIT_FF1, WAIT_FF2, WAIT_ID, WAIT_LEN, WAIT_INS, WAIT_DATA, WAIT_CHK };
    State state;
    uint8_t pkt[64];
    int pktLen;
    int dataCount;

    // Debug
    bool debugEnabled;
    void debugPrint(const char *msg);
    void debugDump(const char *label, const uint8_t *buf, int len);

    // Helpers
    uint8_t checksum(uint8_t *data, int len);
    void parseByte(uint8_t b);
    void sendStatus(uint8_t err, uint8_t *params = nullptr, uint8_t paramLen = 0);
    void updateRing();
    void startupAnimation();
};
