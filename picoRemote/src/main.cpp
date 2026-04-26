/////////////////////////////////////
// PicoRemote — Universal Remote
// Platform: RP2040 (Raspberry Pi Pico), earlephilhower Arduino core
// Protocol: CRSF (bidirectional) with BetaFPV MicroRX ELRS 4.x
//
// Wiring note for telemetry receive:
//   Place a 1 kΩ resistor between Serial1 TX (GPIO 4) and Serial1 RX (GPIO 5).
//   This enables half-duplex operation: the module drives the bus for telemetry
//   frames and the resistor limits current during simultaneous TX.
//   Also confirm the PCB v2.0 solder jumper (bypass TX buffer) is closed.
//
// Channel layout (internal channels[] array, 32 entries):
//   [0..15]  Multiplexer analog channels
//   [16]     Nunchuck X axis
//   [17]     Nunchuck Y axis
//   [18]     Nunchuck buttons
//   [19]     Keypad character (ASCII)
//   [20..23] Switch-stuffed bytes (2 bits per channel, 16 channels packed)
//   [24..31] DMX / encoder / spare
//
// CRSF channels[] → rcChannels[] mapping is per-vehicle in config.h.
//
// Resources:
//   https://arduino-pico.readthedocs.io/en/latest/index.html
//   https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md
//   https://www.expresslrs.org/info/telemetry/

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "crsf_channels.h"

#define NUM_CHANNELS 32
static unsigned int channels[NUM_CHANNELS] = {
    127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// ── CRSF ─────────────────────────────────────────────────────────────────────
#ifdef USE_CRSF
#include "crsf.h"
#endif

// ── OLED display ─────────────────────────────────────────────────────────────
#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, DISPLAY_HEIGHT, &Wire1);
static void processScreen(int mode, float battery, int tracknr);
#endif

// ── Wii Nunchuck ─────────────────────────────────────────────────────────────
#ifdef USE_NUNCHUCK
#include "nunchuck.h"
nunchuck chuck;
#define X_CENTER 130
#define Y_CENTER 127
#endif

// ── USB MIDI host ─────────────────────────────────────────────────────────────
#ifdef USE_USB_MIDI
#include "USBhostfunctions.h"
#endif

// ── DMX input ─────────────────────────────────────────────────────────────────
#ifdef USE_DMX
#include "DmxInput.h"
DmxInput dmxInput;
#define DMX_START_CHANNEL  1
#define DMX_NUM_CHANNELS  16
#define DMX_OFFSET         8
static volatile uint8_t dmxBuffer[DMXINPUT_BUFFER_SIZE(DMX_START_CHANNEL, DMX_NUM_CHANNELS)];
#endif

// ── Multiplexer (fixed hardware, always present) ──────────────────────────────
#include "muxcontrol.h"
static int muxPins[] = {16, 17, 18, 19};
static MuxControl mux(muxPins, 4);

// ── APC220 RF (alternative to CRSF) ──────────────────────────────────────────
#ifdef USE_APC
#include "apcrf.h"
static apcRF myTransmitter(22);
#endif

// ── Keypad ────────────────────────────────────────────────────────────────────
#ifdef USE_KEYPAD
#include "keypad.h"
//                   C1  C2  C3  R1  R2  R3  R4
static int keypadPins[] = {2, 3, 11, 12, 13, 14, 15};
static keypad mypad(keypadPins, 7);
#endif

// ── Battery gauge ─────────────────────────────────────────────────────────────
#ifdef USE_MAX17048
#include "Adafruit_MAX1704X.h"
static Adafruit_MAX17048 maxlipo;
#endif

// ── Quadrature encoder ───────────────────────────────────────────────────────
#ifdef USE_ENCODER
#include <hardware/pio.h>
#include "quadrature.pio.h"
#define QUADRATURE_A_PIN 8
#define QUADRATURE_B_PIN 9
static PIO  encPio = pio0;
static uint encSm;
#endif

// ── Track list (LUMI only) ────────────────────────────────────────────────────
#ifdef LUMI
#define NUM_TRACKS 16
static const char* tracklist[NUM_TRACKS] = {
    "  stop    ", " intro    ", " drome    ", " mazurka  ",
    " scottish ", "spiegelbos", "la pature ", "moeig bent",
    "  wals    ", "de poolkap", "cerkelzaag", " optocht  ",
    "  karlijn ", "zo anders ", "middernach", "  orage   "
};
#endif

// ─────────────────────────────────────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────────────────────────────────────
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

#ifdef USE_CRSF
    crsfInit();  // Serial1 @ 420000 baud, half-duplex
#endif

#ifdef USE_NUNCHUCK
    chuck.begin();
#endif

    mux.initMux();

#ifdef USE_DMX
    dmxInput.begin(0, DMX_START_CHANNEL, DMX_NUM_CHANNELS);
    dmxInput.read_async(dmxBuffer);
#endif

#ifdef USE_ENCODER
    encSm = pio_claim_unused_sm(encPio, true);
    pinMode(QUADRATURE_A_PIN, INPUT_PULLUP);
    pinMode(QUADRATURE_B_PIN, INPUT_PULLUP);
    unsigned int offset = pio_add_program(encPio, &quadratureA_program);
    quadratureA_program_init(encPio, encSm, offset, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
#endif

#ifdef USE_OLED
    Wire1.setSCL(7);
    Wire1.setSDA(6);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
#endif

#ifdef USE_KEYPAD
    mypad.begin();
#endif

#ifdef USE_MAX17048
    while (!maxlipo.begin()) {
        Serial.println(F("MAX17048 not found — check battery connection"));
        delay(2000);
    }
    Serial.print(F("MAX17048 chip ID: 0x"));
    Serial.println(maxlipo.getChipID(), HEX);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN LOOP (Core 0)
// ─────────────────────────────────────────────────────────────────────────────
void loop() {
    static int   tracknr  = 0;
    static float battPct  = 0.0f;

    // ── CRSF TX: poll at full speed, timing managed inside callback ───────────
#ifdef USE_CRSF
    crsfCallback();

    // ── CRSF RX: parse any incoming telemetry bytes ───────────────────────────
    crsfProcessRx();
    // crsfLink and crsfBattery are updated when new frames arrive.
    // crsfLinkUpdated / crsfBatteryUpdated flags are set; clear after reading.
#endif

    // ── 20 Hz main loop ───────────────────────────────────────────────────────
    static unsigned long looptime = 0;
    if (millis() >= looptime + 49) {
        looptime = millis();

        // Heartbeat LED
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

        // ── Sample Wii Nunchuck ───────────────────────────────────────────────
#ifdef USE_NUNCHUCK
        chuck.update(200);
        if (chuck.buttons > 1) {
            channels[16] = 127 + (map(chuck.analogStickX, 23, 215, 0, 255) - X_CENTER) / FAST_MODE;
            channels[17] = 127 + (map(chuck.analogStickY, 29, 224, 0, 255) - Y_CENTER) / FAST_MODE;
        } else {
            channels[16] = 127 + (map(chuck.analogStickX, 23, 215, 0, 255) - X_CENTER) / SLOW_MODE;
            channels[17] = 127 + (map(chuck.analogStickY, 29, 224, 0, 255) - Y_CENTER) / SLOW_MODE;
        }
        channels[18] = chuck.buttons * 64;
#endif

        // ── Sample encoder ────────────────────────────────────────────────────
#ifdef USE_ENCODER
        pio_sm_exec_wait_blocking(encPio, encSm, pio_encode_in(pio_x, 32));
        int encPos = pio_sm_get_blocking(encPio, encSm) / 2 % NUM_TRACKS;
        tracknr = constrain(encPos, 0, NUM_TRACKS - 1);
        channels[ENCODER_CHANNEL] = tracknr * 8;
#endif

        // ── Sample multiplexer analog channels ────────────────────────────────
        for (int i = 0; i < 16; i++) {
            channels[i] = mux.checkMux(i);
            if (invertChannel[i]) channels[i] = 255 - channels[i];
        }
#ifdef DEBUG_MUX
        // Print all 16 mux raw values — enable to diagnose wiring/hardware issues
        Serial.print("MUX:");
        for (int i = 0; i < 16; i++) {
            Serial.print(" ch");
            Serial.print(i);
            Serial.print("=");
            Serial.print(channels[i]);
        }
        Serial.println();
#endif

        // ── Sample DMX ────────────────────────────────────────────────────────
#ifdef USE_DMX
        if (millis() > dmxInput.latest_packet_timestamp() + 100) {
            // No DMX signal: load safe defaults
            for (int i = 0; i < 8; i++) channels[i + 24] = 0;
            channels[24] = 127;
            channels[25] = 20;
            channels[26] = 64;
        } else {
            for (int i = 0; i < 8; i++)
                channels[i + 19] = dmxBuffer[i + 1 + DMX_OFFSET];
        }
#endif

        // ── Sample keypad ─────────────────────────────────────────────────────
#ifdef USE_KEYPAD
        // Store full 12-bit bitmask: low byte in [19], high nibble in [25]
        uint16_t keyBits = mypad.getKeypad();
        channels[19] = keyBits & 0xFF;         // bits 0–7
        channels[25] = (keyBits >> 8) & 0x0F;  // bits 8–11
#endif

        // ── Pack switch channels into 4 bytes, 2 bits per mux channel ──────────
        // Bit position for mux channel N: bits (2*(N%4)) and (2*(N%4)+1)
        // in switch bank N/4. Values: 0b00=mid, 0b01=low, 0b10=high.
        // See crsf_channels.h for SWITCH_STATE() decode macro.
#ifdef STUFF_SWITCHES
        uint8_t swBank[4] = {0, 0, 0, 0};
        for (int i = 0; i < 16; i++) {
            if (switchChannel[i] > 0) {
                uint8_t state = 0;                    // default: mid
                if (channels[i] <  64) state = 0x01; // low
                if (channels[i] > 191) state = 0x02; // high
                swBank[i / 4] |= (state << ((i % 4) * 2));
            }
        }
        channels[20] = swBank[0];  // mux ch 0–3
        channels[21] = swBank[1];  // mux ch 4–7
        channels[22] = swBank[2];  // mux ch 8–11
        channels[23] = swBank[3];  // mux ch 12–15
#endif

        // ── Battery from MAX17048 gauge ───────────────────────────────────────
#ifdef USE_MAX17048
        float cellV = maxlipo.cellVoltage();
        if (!isnan(cellV)) {
            battPct = maxlipo.cellPercent();
        }
#endif

        // ── Debug serial dump ─────────────────────────────────────────────────
#ifdef DEBUG
        for (int i = 0; i < 24; i++) {
            Serial.print(channels[i]);
            Serial.print(',');
        }
#ifdef USE_CRSF
        Serial.print(" ARM:");    Serial.print(channels[ARM_MUX_CHANNEL] > 128 ? "ON" : "off");
        // Telemetry health — if rxBytes stays 0, no bytes at all are coming back.
        Serial.print(" rxB:");    Serial.print(crsfRxBytes);
        Serial.print(" ok:");     Serial.print(crsfFramesOk);
        Serial.print(" crcErr:"); Serial.print(crsfFramesBadCrc);
        Serial.print(" RSSI:");   Serial.print((int)crsfLink.rssi_ant1);      // already signed dBm
        Serial.print(" LQ:");     Serial.print(crsfLink.link_quality);
#endif
        Serial.println();
#endif

        // ── Map to CRSF channels — fixed protocol layout (see crsf_channels.h) ──
#ifdef USE_CRSF
        // Helper: map 0–255 to CRSF range, or send mid if source = AXIS_NOT_CONNECTED
        auto crsf_val = [](unsigned int src) -> int {
            return map(src, 0, 255, CRSF_CHANNEL_MIN, CRSF_CHANNEL_MAX);
        };
        auto crsf_src = [&](int src_idx) -> int {
            if (src_idx == AXIS_NOT_CONNECTED) return CRSF_CHANNEL_MID;
            return crsf_val(channels[src_idx]);
        };

        rcChannels[CRSF_CH_AXIS_X1]      = crsf_src(AXIS_X1_SRC);
        rcChannels[CRSF_CH_AXIS_Y1]      = crsf_src(AXIS_Y1_SRC);
        rcChannels[CRSF_CH_AXIS_X2]      = crsf_src(AXIS_X2_SRC);
        rcChannels[CRSF_CH_AXIS_Y2]      = crsf_src(AXIS_Y2_SRC);
        rcChannels[CRSF_CH_ARM]          = (channels[ARM_MUX_CHANNEL] > 128)
                                           ? CRSF_CHANNEL_MAX : CRSF_CHANNEL_MIN;
        rcChannels[CRSF_CH_ANALOG1]      = crsf_src(ANALOG1_SRC);
        rcChannels[CRSF_CH_ANALOG2]      = crsf_src(ANALOG2_SRC);
        rcChannels[CRSF_CH_ANALOG3]      = crsf_src(ANALOG3_SRC);
#ifdef USE_NUNCHUCK
        rcChannels[CRSF_CH_NUNCHUCK_BTN] = crsf_val(channels[18]);
#else
        rcChannels[CRSF_CH_NUNCHUCK_BTN] = CRSF_CHANNEL_MID;
#endif
#ifdef USE_KEYPAD
        rcChannels[CRSF_CH_KEYPAD_LO]    = crsf_val(channels[19]);  // bits 0–7
        rcChannels[CRSF_CH_KEYPAD_HI]    = crsf_val(channels[25]);  // bits 8–11
#else
        rcChannels[CRSF_CH_KEYPAD_LO]    = CRSF_CHANNEL_MIN;
        rcChannels[CRSF_CH_KEYPAD_HI]    = CRSF_CHANNEL_MIN;
#endif
#ifdef STUFF_SWITCHES
        rcChannels[CRSF_CH_SW_MUX_0_3]   = crsf_val(channels[20]);
        rcChannels[CRSF_CH_SW_MUX_4_7]   = crsf_val(channels[21]);
        rcChannels[CRSF_CH_SW_MUX_8_11]  = crsf_val(channels[22]);
        rcChannels[CRSF_CH_SW_MUX_12_15] = crsf_val(channels[23]);
#else
        rcChannels[CRSF_CH_SW_MUX_0_3]   = CRSF_CHANNEL_MIN;
        rcChannels[CRSF_CH_SW_MUX_4_7]   = CRSF_CHANNEL_MIN;
        rcChannels[CRSF_CH_SW_MUX_8_11]  = CRSF_CHANNEL_MIN;
        rcChannels[CRSF_CH_SW_MUX_12_15] = CRSF_CHANNEL_MIN;
#endif
        rcChannels[CRSF_CH_SPARE]         = CRSF_CHANNEL_MID;
#else
        // APC220 legacy path — sends first 16 channels in fixed protocol order
        static unsigned char txMsg[16];
        txMsg[0]  = channels[AXIS_X1_SRC  == AXIS_NOT_CONNECTED ? 0 : AXIS_X1_SRC];
        txMsg[1]  = channels[AXIS_Y1_SRC  == AXIS_NOT_CONNECTED ? 0 : AXIS_Y1_SRC];
        txMsg[2]  = channels[18];  // nunchuck buttons
        txMsg[3]  = channels[19];  // keypad lo
        txMsg[4]  = channels[ANALOG1_SRC == AXIS_NOT_CONNECTED ? 0 : ANALOG1_SRC];
        txMsg[5]  = channels[20];  // switch bank A
        txMsg[6]  = channels[21];  // switch bank B
        txMsg[7]  = channels[22];  // switch bank C
        txMsg[8]  = channels[23];  // switch bank D
        for (int i = 9; i < 16; i++) txMsg[i] = 127;
        myTransmitter.RobotWrite(13, txMsg, 16);
#endif
    } // end 20 Hz block

    // ── Display update at 10 Hz ───────────────────────────────────────────────
#ifdef USE_OLED
    static unsigned long screentime = 0;
    if (millis() >= screentime + 99) {
        screentime = millis();
#ifdef LUMI
        processScreen(1, battPct, tracknr);
#else
        processScreen(3, battPct, tracknr);
#endif
    }
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// CORE 1: USB MIDI host
// ─────────────────────────────────────────────────────────────────────────────
void setup1() {
#ifdef USE_USB_MIDI
    Serial.printf("Core1: TinyUSB host starting\r\n");
    uint32_t cpu_hz = clock_get_hz(clk_sys);
    if (cpu_hz != 120000000UL && cpu_hz != 240000000UL) {
        delay(2000);
        Serial.printf("ERROR: CPU clock %u Hz — PIO USB needs 120 or 240 MHz\r\n", cpu_hz);
        while (1) delay(1);
    }
    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = PIN_USB_HOST_DP;
    USBHost.configure_pio_usb(1, &pio_cfg);
#ifdef PIN_5V_EN
    pinMode(PIN_5V_EN, OUTPUT);
    digitalWrite(PIN_5V_EN, PIN_5V_EN_STATE);
#endif
    USBHost.begin(1);
#endif
}

void loop1() {
#ifdef USE_USB_MIDI
    USBHost.task();
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// OLED DISPLAY  (128×32 pixels for most vehicles)
//
// Layout (all modes share the right column for status):
//
//  [0..19] Joystick 1   [20..43] Joystick 2   [44..83] Switches+Keypad   [84..127] Status
//  ┌────────────────────────────────────────────────────────────────────────────────────────┐
//  │ (J1 circle 20×20) │ (J2 circle 20×20) │ sw ■□■■ kp dots           │ SCUBA  🔋62%    │
//  │                   │                   │                             │ LQ:98%          │
//  │                   │                   │                             │ ▂▄▆ -64dBm      │
//  │                   │                   │                             │                 │
//  └────────────────────────────────────────────────────────────────────────────────────────┘
//
// ─────────────────────────────────────────────────────────────────────────────
#ifdef USE_OLED

// Draw a small joystick indicator: circle with crosshair and dot showing position.
// cx,cy = centre, r = radius. invertY = true flips the Y axis (nunchuck convention).
static void drawJoystick(int cx, int cy, int r, int xVal, int yVal, bool invertY = false) {
    display.drawCircle(cx, cy, r, SSD1306_WHITE);
    display.drawLine(cx, cy - r + 1, cx, cy + r - 1, SSD1306_WHITE);
    display.drawLine(cx - r + 1, cy, cx + r - 1, cy, SSD1306_WHITE);
    int dx = map(xVal, 0, 255, cx - r + 2, cx + r - 2);
    int dy = invertY
             ? map(yVal, 0, 255, cy + r - 2, cy - r + 2)   // flipped
             : map(yVal, 0, 255, cy - r + 2, cy + r - 2);
    display.fillCircle(dx, dy, 2, SSD1306_WHITE);
}

// Draw antenna-style signal strength bars (3 bars, bottom-anchored at y).
// rssi_dbm is a negative value: -40 dBm or better = 3 bars, -100 = 0 bars.
static void drawSignalBars(int x, int y, int rssi_dbm) {
    int strength = constrain(map(rssi_dbm, -100, -40, 0, 3), 0, 3);
    int barW = 3, gap = 1;
    int heights[3] = {3, 5, 7};
    for (int i = 0; i < 3; i++) {
        int bx = x + i * (barW + gap);
        int bh = heights[i];
        if (i < strength) {
            display.fillRect(bx, y - bh + 1, barW, bh, SSD1306_WHITE);
        } else {
            display.drawRect(bx, y - bh + 1, barW, bh, SSD1306_WHITE);
        }
    }
}

// Draw tiny battery symbol (10×5px body + nub) with fill and percentage text.
static void drawBattery(int x, int y, float pct) {
    display.drawRect(x, y, 10, 5, SSD1306_WHITE);
    display.drawRect(x + 10, y + 1, 2, 3, SSD1306_WHITE);
    int fill = constrain((int)(pct / 100.0f * 8), 0, 8);
    if (fill > 0) display.fillRect(x + 1, y + 1, fill, 3, SSD1306_WHITE);
    display.setCursor(x + 13, y);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print((int)pct);
    display.print("%");
}

// Draw volume as a horizontal bar: outline + filled portion, label "v" at left.
// x,y = left edge of the label, val = 0–255.
static void drawVolBar(int x, int y, int val) {
    display.setCursor(x, y);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("v");
    int bx = x + 7, bw = 30, bh = 4;
    display.drawRect(bx, y, bw, bh, SSD1306_WHITE);
    int fill = map(val, 0, 255, 0, bw - 2);
    if (fill > 0) display.fillRect(bx + 1, y + 1, fill, bh - 2, SSD1306_WHITE);
}

// Draw switches as a 4×4 grid (16 switches, mux ch 0–15).
// Cell size 5×5px. Each cell always shows:
//   low  (0): centre pixel dot
//   mid  (1): open 3×3 square (SPDT centre)
//   high (2): filled 4×4 square
static void drawSwitches(int x, int y, uint8_t sw[4]) {
    for (int i = 0; i < 16; i++) {
        int state = SWITCH_STATE(sw, i);
        int col = i % 4;
        int row = i / 4;
        int px = x + col * 5;
        int py = y + row * 5;
        if (state == 2) {
            display.fillRect(px, py, 4, 4, SSD1306_WHITE);
        } else if (state == 1) {
            display.drawRect(px, py, 4, 4, SSD1306_WHITE);
        } else {
            display.drawPixel(px + 2, py + 2, SSD1306_WHITE);
        }
    }
}

// Draw keypad as a 4×3 grid (12 keys, matching physical layout).
// Cell size 4×5px. Filled = pressed, centre pixel = unpressed.
// Physical layout: col0=C1(1,4,7,*) col1=C2(2,5,8,0) col2=C3(3,6,9,#)
// keypad.cpp scan order: bit0=1, 1=4, 2=7, 3=*, 4=2, 5=5, 6=8, 7=0, 8=3, 9=6, 10=9, 11=#
static void drawKeypad(int x, int y, uint16_t keyBits) {
    for (int i = 0; i < 12; i++) {
        int col = i / 4;    // physical column (0=left, 1=mid, 2=right)
        int row = i % 4;    // physical row (0=top, 3=bottom)
        int px = x + col * 5;
        int py = y + row * 4;
        if ((keyBits >> i) & 1) {
            display.fillRect(px, py, 4, 3, SSD1306_WHITE);
        } else {
            display.drawPixel(px + 2, py + 1, SSD1306_WHITE);
        }
    }
}

// ── Layout (128×32px) ─────────────────────────────────────────────────────────
//
//  y=0–7   [NAME(0)] [🔋xx%(38)] [LQ:xx%(90)]         — top bar, full width
//  y=9–28  x=0–19   J1 circle r=9 centre(10,19)
//          x=21     C/Z nunchuck buttons (two 3×3 dots, y=15 and y=22)
//  y=9–28  x=24–43  J2 circle r=9 centre(34,19)
//  y=9–28  x=44–63  Switches 4×4 grid (5px cell = 20×20px)
//  y=9–28  x=65–79  Keypad   3×4 grid (4px col × 5px row = 12×20px... wait
//                    actually 4 rows × 3 cols: cell 5×4 = 15×16px at y=12)
//  y=9–14  x=84–127 Vol bar (label "v" + 30px bar)
//  y=16–23 x=84–127 Signal bars (3 bars) + dBm text
//
static void processScreen(int mode, float battery, int tracknr) {
    display.clearDisplay();

#ifdef DISPLAY_ROTATE
    display.setRotation(2);
#endif

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // ── Top row (y=0): NAME | battery | LQ ───────────────────────────────────
    // Text size 1 = 6px wide per char, 8px tall.
    // NAME: max 6 chars = 36px. Battery: 12px symbol + ~18px text = 30px. LQ: ~24px.
    // Layout: NAME(x=0) | bat(x=38) | LQ(x=84) — LQ needs <=7 chars to fit before x=127
    display.setCursor(0, 0);
#if defined(SCUBA)
    display.print("SCUBA");
#elif defined(AMI)
    display.print("AMI");
#elif defined(LUMI)
    display.print("LUMI");
#elif defined(ANIMALTRONIEK_KREEFT)
    display.print("KREEFT");
#elif defined(ANIMALTRONIEK_VIS)
    display.print("VIS");
#elif defined(ANIMAL_LOVE)
    display.print("LOVE");
#elif defined(EXPERIMENTAL)
    display.print("EXPMT");
#else
    display.print("?");
#endif

    if (battery > 0.0f) drawBattery(38, 0, battery);

#ifdef USE_CRSF
    uint8_t lq   = crsfLink.link_quality;
    int     rssi = (int)crsfLink.rssi_ant1;
    // LQ at x=84: "LQ:" (3 chars=18px) + up to 3 digit number + "%" = max 7 chars = 42px → fits
    display.setCursor(84, 0);
    display.print("LQ:");
    display.print(lq);
    display.print("%");
#endif

    // ── LUMI encoder track (64px display only) ────────────────────────────────
#ifdef USE_ENCODER
    if (mode == 1 && tracknr >= 0) {
        display.setTextSize(2);
        display.fillRect(0, 47, 12 * strlen(tracklist[tracknr]) + 2, 20, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(0, 48);
        display.print(tracklist[tracknr]);
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
    }
#endif

    // ── Joystick 1 (centre x=10, y=19, r=9) — Y inverted for nunchuck ────────
    {
        int x1 = (AXIS_X1_SRC == AXIS_NOT_CONNECTED) ? 127 : channels[AXIS_X1_SRC];
        int y1 = (AXIS_Y1_SRC == AXIS_NOT_CONNECTED) ? 127 : channels[AXIS_Y1_SRC];
#ifdef USE_NUNCHUCK
        drawJoystick(10, 19, 9, x1, y1, true);   // nunchuck Y is inverted
#else
        drawJoystick(10, 19, 9, x1, y1, false);
#endif
    }

    // ── Nunchuck C/Z buttons — shown only when pressed, right of J1 at x=21 ──
    // nunchuck.h: buttons = 3 - (data[5] & 0x03), where data[5] bit0=Z(active low),
    // bit1=C(active low). Result: Z only=1, C only=2, both=3. Multiplied by 64:
    // Z=64, C=128, both=192. Use value comparison, not bitmask, to be unambiguous.
#ifdef USE_NUNCHUCK
    {
        uint8_t btn = channels[18];
        bool zPressed = (btn == 64 || btn == 192);   // Z only, or both
        bool cPressed = (btn == 128 || btn == 192);  // C only, or both
        if (zPressed) display.fillRect(21, 13, 3, 3, SSD1306_WHITE); // Z: upper dot
        if (cPressed) display.fillRect(21, 19, 3, 3, SSD1306_WHITE); // C: lower dot
    }
#endif

    // ── Joystick 2 (centre x=34, y=19, r=9) ──────────────────────────────────
    {
        if (AXIS_X2_SRC == AXIS_NOT_CONNECTED) {
            display.drawCircle(34, 19, 9, SSD1306_WHITE);
            display.drawLine(34, 11, 34, 27, SSD1306_WHITE);
            display.drawLine(26, 19, 42, 19, SSD1306_WHITE);
        } else {
            drawJoystick(34, 19, 9, channels[AXIS_X2_SRC], channels[AXIS_Y2_SRC], false);
        }
    }

    // ── Switches: 4×4 grid at x=44, y=11 (4 cols × 4 rows, 5px cell = 20×20px)
#ifdef STUFF_SWITCHES
    {
        uint8_t sw[4] = {
            channels[20], channels[21], channels[22], channels[23]
        };
        drawSwitches(44, 11, sw);
    }
#endif

    // ── Keypad: 3×4 grid at x=65, y=11 (3 cols × 4 rows, 5×5px = 15×20px) ───
#ifdef USE_KEYPAD
    {
        uint16_t keyBits = (uint16_t)channels[19] |
                           ((uint16_t)(channels[25] & 0x0F) << 8);
        drawKeypad(65, 11, keyBits);
    }
#endif

    // ── Right column (x=84–127): signal bars upper, vol bar lower ──────────────
    // Signal bars at y=9–15 (bars bottom-anchored at y=15), dBm text y=9
#ifdef USE_CRSF
    drawSignalBars(84, 15, rssi);
    display.setCursor(97, 9);
    display.print(rssi);
    display.print("d");
#endif

    // Vol bar at bottom (y=24–28) — read directly from channels[], not gated on CRSF
#if ANALOG1_SRC != 255
    drawVolBar(84, 24, channels[ANALOG1_SRC]);
#endif

    display.display();
}

#endif // USE_OLED