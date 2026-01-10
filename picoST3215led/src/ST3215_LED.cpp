
#include "ST3215_LED.h"


// ===== Constructor =====
ST3215_LED::ST3215_LED(uint8_t uartId,
                       uint8_t txPin,
                       uint8_t rxPin,
                       uint8_t srPin,
                       uint8_t ledPin,
                       uint16_t ledCount,
                       uint8_t id)
: uartId(uartId), txPin(txPin), rxPin(rxPin), srPin(srPin),
  ledPin(ledPin), ledCount(ledCount),
  strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800),
  myID(id), pattern(0), colour(0), brightness(0), state(WAIT_FF1), pktLen(0), dataCount(0),
  debugEnabled(false)
{}

// ===== Public =====
void ST3215_LED::begin(bool debug) {
    debugEnabled = debug;
    if (debugEnabled) {
        Serial.begin(115200);
        //while (!Serial) delay(10);
        debugPrint("ST3215_LED starting...");
    }
    pinMode(srPin,OUTPUT);
    strip.begin();
    strip.show();
    startupAnimation();

    if (uartId == 0) {
        uart_init(uart0, 1000000);
    } else {
        uart_init(uart1, 1000000);
    }
    gpio_set_function(txPin, GPIO_FUNC_UART);
    gpio_set_function(rxPin, GPIO_FUNC_UART);

    if (debugEnabled) debugPrint("UART initialized at 1 Mbps");
}

void ST3215_LED::update() {
    uart_inst_t *uart = (uartId == 0) ? uart0 : uart1;
    while (uart_is_readable(uart)) {
        uint8_t b = uart_getc(uart);
        parseByte(b);
    }
}

void ST3215_LED::setID(uint8_t newID) {
    myID = newID;
    if (debugEnabled) {
        Serial.print("ID changed to ");
        Serial.println(myID);
    }
}

// ===== Debug helpers =====
void ST3215_LED::debugPrint(const char *msg) {
    if (debugEnabled) {
        Serial.println(msg);
    }
}

void ST3215_LED::debugDump(const char *label, const uint8_t *buf, int len) {
    if (!debugEnabled) return;
    Serial.print(label);
    for (int i = 0; i < len; i++) {
        if (i % 16 == 0) Serial.print("\n  ");
        if (buf[i] < 16) Serial.print("0");
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// ===== Private =====
uint8_t ST3215_LED::checksum(uint8_t *data, int len) {
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) sum += data[i];
    return ~((uint8_t)(sum & 0xFF));
}

void ST3215_LED::sendStatus(uint8_t err, uint8_t *params, uint8_t paramLen) {
    uart_inst_t *uart = (uartId == 0) ? uart0 : uart1;
    uint8_t buf[32];
    buf[0] = 0xFF;
    buf[1] = 0xFF;
    buf[2] = myID;
    buf[3] = paramLen + 2; // ERR + params + checksum
    buf[4] = err;
    for (int i = 0; i < paramLen; i++) buf[5 + i] = params[i];
    uint8_t chk = checksum(&buf[2], 3 + paramLen);
    buf[5 + paramLen] = chk;
    digitalWrite(srPin,HIGH);
    for (int i = 0; i < 6 + paramLen; i++) {
        uart_putc_raw(uart, buf[i]);
    }
    uart_tx_wait_blocking(uart); // similar to Serial.flush()
    digitalWrite(srPin,LOW);
    debugDump("TX Status", buf, 6 + paramLen);
}

void ST3215_LED::updateRing() {
    strip.clear();
    uint16_t hue = map(colour, 0, 4095, 0, 65532);
    uint16_t bright = map(brightness,0,4095,0,255);
    uint32_t color = strip.ColorHSV(hue, 255, bright);
    for(int i = 0; i<ledCount; i++){
    strip.setPixelColor(i, color);
    }
    strip.show();

    if (debugEnabled) {
        Serial.print("LED updated: goalPos=");
        Serial.print(pattern);
        Serial.print("color: ");
        Serial.print(hue);
        Serial.print(" -> LED ");
        Serial.println(brightness);
    }
}

void ST3215_LED::startupAnimation() {
    if (!debugEnabled) delay(100); // brief pause if no serial
    for (int j = 0; j < 256; j += 8) { // rainbow wipe
        for (int i = 0; i < ledCount; i++) {
            uint32_t color = strip.ColorHSV(((i * 65536L / ledCount) + j * 256) & 0xFFFF, 255, 64);
            strip.setPixelColor(i, color);
        }
        strip.show();
        delay(30);
    }
    strip.clear();
    strip.show();
    debugPrint("Startup animation done");
}

void ST3215_LED::parseByte(uint8_t b) {
    switch (state) {
        case WAIT_FF1:
            if (b == 0xFF) state = WAIT_FF2;
            break;
        case WAIT_FF2:
            if (b == 0xFF) state = WAIT_ID;
            else state = WAIT_FF1;
            break;
        case WAIT_ID:
            pkt[0] = b; 
            //if(b!=14) state = WAIT_FF1;
            state = WAIT_LEN; break;
        case WAIT_LEN:
            pkt[1] = b; pktLen = b; dataCount = 0; state = WAIT_INS; break;
        case WAIT_INS:
            pkt[2] = b;
            if (pktLen > 2) state = WAIT_DATA;
            else state = WAIT_CHK;
            break;
        case WAIT_DATA:
            pkt[3 + dataCount++] = b;
            if (dataCount >= pktLen - 2) state = WAIT_CHK;
            break;
        case WAIT_CHK: {
            uint8_t chk = checksum(pkt, pktLen + 1);
            if (chk == b) {
                debugDump("RX Packet", pkt, pktLen + 1);

                uint8_t id = pkt[0];
                uint8_t ins = pkt[2];

                if (id == myID || id == 0xFE) {
                    if (ins == 0x01) { // PING
                        sendStatus(0x00);
                    }
                    else if (ins == 0x02) { // READ_DATA
                        uint8_t addr = pkt[3];
                        uint8_t len = pkt[4];
                        if (addr == 0x38 && len == 2) {
                            uint8_t resp[2] = {
                                (uint8_t)(goalPos & 0xFF),
                                (uint8_t)(goalPos >> 8)
                            };
                            sendStatus(0x00, resp, 2);
                        }
                    }
                    else if (ins == 0x03) { // WRITE_DATA
                        uint8_t addr = pkt[3];
                        if (addr == 0x05) { // change ID
                            myID = pkt[4];
                            sendStatus(0x00);
                        } else if (addr == 0x29) { // goal pos
                            pattern = pkt[5] | (pkt[6] << 8);
                            colour =  pkt[9] | (pkt[10] << 8);
                            brightness = pkt[5] | (pkt[6] << 8);
                            
                            sendStatus(0x00);
                            updateRing();
                        }
                    }
                }
            }
            state = WAIT_FF1;
            break;
        }
    }
}
