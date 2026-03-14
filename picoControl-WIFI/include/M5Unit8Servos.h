#ifndef M5UNIT8SERVOS_H
#define M5UNIT8SERVOS_H

#include <Arduino.h>
#include <Wire.h>

/**
 * Driver for M5Stack Unit 8Servos (STM32F030, I2C addr 0x25).
 * Uses the official I2C register map from the M5 docs.
 */
class M5Unit8Servos {
public:
    // Default 7-bit I2C address of the unit
    static const uint8_t DEFAULT_ADDR = 0x25;

    // Per-channel IO mode (MODE_SETTING, value 0~5)
    enum ChannelMode : uint8_t {
        MODE_INPUT  = 0, // digital input
        MODE_OUTPUT = 1, // digital output
        MODE_ADC    = 2, // analog input
        MODE_SERVO  = 3, // servo control
        MODE_RGB    = 4, // RGB LED control
        MODE_PWM    = 5  // PWM duty control
    };

    M5Unit8Servos();

    // Initialize with given Wire instance and I2C address.
    // Call this once in setup().
    bool begin(TwoWire &wire = Wire, uint8_t addr = DEFAULT_ADDR);

    // ----- Config / status -----

    // Set IO mode for a single channel (0..7)
    bool setChannelMode(uint8_t channel, ChannelMode mode);

    // Read IO mode of a single channel (0..7)
    int  getChannelMode(uint8_t channel);

    // Change unit's I2C address (0..127). Also updates internal _addr.
    bool setI2CAddress(uint8_t newAddr);

    // Read firmware status/version byte (reg 0xFE by default)
    int  readStatus(uint8_t reg = 0xFE);

    // ----- Arduino-like attach/detach -----

    // Equivalent to Servo.attach(): set channel to MODE_SERVO
    bool attach(uint8_t channel);

    // Equivalent to Servo.detach():
    // By default switches channel to MODE_INPUT (high-impedance).
    // You may pass MODE_OUTPUT or another non-SERVO mode if desired.
    bool detach(uint8_t channel, ChannelMode newMode = MODE_INPUT);

    // ----- Servo control -----

    // Set servo angle in degrees (0..180).
    // autoMode=true makes sure channel is in MODE_SERVO (calls attach()).
    bool writeServoAngle(uint8_t channel, uint8_t angle, bool autoMode = true);

    // Read servo angle in degrees (0..180).
    int  readServoAngle(uint8_t channel);

    // Set servo pulse width in microseconds (500..2500 us recommended).
    // autoMode=true makes sure channel is in MODE_SERVO (calls attach()).
    bool writeServoPulse(uint8_t channel, uint16_t pulseUs, bool autoMode = true);

    // Read servo pulse width in microseconds.
    int  readServoPulse(uint8_t channel);

    // Read total servo current in amps (float from 0xA0..0xA3).
    float readServoCurrent();

    // ----- RGB LED -----

    // Set RGB LED color for a channel, 0xRRGGBB.
    // Mode must be MODE_RGB for that channel.
    bool writeRgbLed(uint8_t channel, uint32_t rgb);

    // Read RGB LED color (0xRRGGBB). Returns -1 on error.
    int32_t readRgbLed(uint8_t channel);

    // ----- Digital / PWM (optional helpers) -----

    // Digital output (0 or 1) when channel is MODE_OUTPUT.
    bool writeOutputPin(uint8_t channel, uint8_t level);

    // Digital input (returns 0/1) when channel is MODE_INPUT.
    int  readInputPin(uint8_t channel);

    // PWM duty (0..100) when channel is MODE_PWM.
    bool writePwmDuty(uint8_t channel, uint8_t duty);

private:
    // Register bases from the official I2C map
    static const uint8_t REG_MODE_BASE       = 0x00; // per-channel mode
    static const uint8_t REG_OUTPUT_BASE     = 0x10; // digital out
    static const uint8_t REG_INPUT_DIGITAL   = 0x20; // digital in
    static const uint8_t REG_ADC8_BASE       = 0x30; // 8-bit ADC
    static const uint8_t REG_ADC12_BASE      = 0x40; // 12-bit ADC (low/high)
    static const uint8_t REG_SERVO_ANGLE     = 0x50; // 8-bit angle
    static const uint8_t REG_SERVO_PULSE     = 0x60; // 16-bit pulse (L/H)
    static const uint8_t REG_RGB_BASE        = 0x70; // 24-bit RGB
    static const uint8_t REG_PWM_DUTY_BASE   = 0x90; // PWM duty
    static const uint8_t REG_SERVO_CURRENT   = 0xA0; // 4-byte float
    static const uint8_t REG_I2C_ADDR        = 0xF0; // new I2C address

    TwoWire *_wire;
    uint8_t  _addr;
    bool     _begun;

    bool writeReg(uint8_t reg, uint8_t value);
    bool writeRegMulti(uint8_t reg, const uint8_t *data, size_t len);
    int  readReg(uint8_t reg);               // read one byte, -1 on error
    bool readRegMulti(uint8_t reg, uint8_t *data, size_t len);
};

#endif // M5UNIT8SERVOS_H
