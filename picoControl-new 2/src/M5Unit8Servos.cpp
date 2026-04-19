#include "M5Unit8Servos.h"

M5Unit8Servos::M5Unit8Servos()
    : _wire(nullptr), _addr(DEFAULT_ADDR), _begun(false) {}

bool M5Unit8Servos::begin(TwoWire &wire, uint8_t addr) {
    _wire  = &wire;
    _addr  = addr;
    _wire->begin();
    _begun = true;
    return true;
}

// ---------- low-level I2C helpers ----------

bool M5Unit8Servos::writeReg(uint8_t reg, uint8_t value) {
    if (!_begun) return false;
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->write(value);
    return (_wire->endTransmission() == 0);
}

bool M5Unit8Servos::writeRegMulti(uint8_t reg, const uint8_t *data, size_t len) {
    if (!_begun) return false;
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if (len && data) _wire->write(data, len);
    return (_wire->endTransmission() == 0);
}

int M5Unit8Servos::readReg(uint8_t reg) {
    if (!_begun) return -1;
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0) return -1;
    if (_wire->requestFrom((int)_addr, 1) != 1) return -1;
    return _wire->read();
}

bool M5Unit8Servos::readRegMulti(uint8_t reg, uint8_t *data, size_t len) {
    if (!_begun || !data || !len) return false;
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0) return false;
    size_t n = _wire->requestFrom((int)_addr, (int)len);
    if (n != len) return false;
    for (size_t i = 0; i < len; ++i) {
        data[i] = _wire->read();
    }
    return true;
}

// ---------- config / status ----------

bool M5Unit8Servos::setChannelMode(uint8_t channel, ChannelMode mode) {
    if (channel > 7) return false;
    return writeReg(REG_MODE_BASE + channel, (uint8_t)mode);
}

int M5Unit8Servos::getChannelMode(uint8_t channel) {
    if (channel > 7) return -1;
    return readReg(REG_MODE_BASE + channel);
}

bool M5Unit8Servos::setI2CAddress(uint8_t newAddr) {
    if (newAddr > 0x7F) return false;
    bool ok = writeReg(REG_I2C_ADDR, newAddr);
    if (ok) {
        _addr = newAddr;
    }
    return ok;
}

int M5Unit8Servos::readStatus(uint8_t reg) {
    return readReg(reg); // UIFlow uses 0xFE for FW version / status
}

// ---------- Arduino-like attach/detach ----------

bool M5Unit8Servos::attach(uint8_t channel) {
    // Equivalent to Servo.attach(): enable servo mode
    return setChannelMode(channel, MODE_SERVO);
}

bool M5Unit8Servos::detach(uint8_t channel, ChannelMode newMode) {
    // Prevent nonsense: don't detach to MODE_SERVO
    if (newMode == MODE_SERVO) {
        newMode = MODE_INPUT;  // default to high-impedance
    }
    return setChannelMode(channel, newMode);
}

// ---------- servo control ----------

bool M5Unit8Servos::writeServoAngle(uint8_t channel, uint8_t angle, bool autoMode) {
    if (channel > 7) return false;
    if (angle > 180) angle = 180;
    if (autoMode) {
        attach(channel);
    }
    return writeReg(REG_SERVO_ANGLE + channel, angle);
}

int M5Unit8Servos::readServoAngle(uint8_t channel) {
    if (channel > 7) return -1;
    return readReg(REG_SERVO_ANGLE + channel);
}

bool M5Unit8Servos::writeServoPulse(uint8_t channel, uint16_t pulseUs, bool autoMode) {
    if (channel > 7) return false;
    // Datasheet recommends range 500~2500 us
    if (pulseUs < 500)  pulseUs = 500;
    if (pulseUs > 2500) pulseUs = 2500;

    if (autoMode) {
        attach(channel);
    }

    uint8_t buf[2];
    buf[0] = pulseUs & 0xFF;         // low byte
    buf[1] = (pulseUs >> 8) & 0xFF;  // high byte

    uint8_t reg = REG_SERVO_PULSE + (channel * 2);
    return writeRegMulti(reg, buf, 2);
}

int M5Unit8Servos::readServoPulse(uint8_t channel) {
    if (channel > 7) return -1;
    uint8_t buf[2];
    uint8_t reg = REG_SERVO_PULSE + (channel * 2);
    if (!readRegMulti(reg, buf, 2)) return -1;
    uint16_t pulse = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    return (int)pulse;
}

float M5Unit8Servos::readServoCurrent() {
    uint8_t buf[4];
    if (!readRegMulti(REG_SERVO_CURRENT, buf, 4)) {
        return NAN;
    }
    // STM32 is little-endian → float bytes in little-endian order
    float value;
    memcpy(&value, buf, 4);
    return value;
}

// ---------- RGB LED ----------

bool M5Unit8Servos::writeRgbLed(uint8_t channel, uint32_t rgb) {
    if (channel > 7) return false;
    // make sure in RGB mode
    setChannelMode(channel, MODE_RGB);

    uint8_t buf[3];
    buf[0] = (rgb >> 16) & 0xFF; // R
    buf[1] = (rgb >> 8) & 0xFF;  // G
    buf[2] = rgb & 0xFF;         // B

    uint8_t reg = REG_RGB_BASE + (channel * 3);
    return writeRegMulti(reg, buf, 3);
}

int32_t M5Unit8Servos::readRgbLed(uint8_t channel) {
    if (channel > 7) return -1;
    uint8_t buf[3];
    uint8_t reg = REG_RGB_BASE + (channel * 3);
    if (!readRegMulti(reg, buf, 3)) return -1;
    uint32_t rgb = ((uint32_t)buf[0] << 16) |
                   ((uint32_t)buf[1] << 8)  |
                   (uint32_t)buf[2];
    return (int32_t)rgb;
}

// ---------- digital / PWM helpers ----------

bool M5Unit8Servos::writeOutputPin(uint8_t channel, uint8_t level) {
    if (channel > 7) return false;
    setChannelMode(channel, MODE_OUTPUT);
    level = (level ? 1 : 0);
    return writeReg(REG_OUTPUT_BASE + channel, level);
}

int M5Unit8Servos::readInputPin(uint8_t channel) {
    if (channel > 7) return -1;
    setChannelMode(channel, MODE_INPUT);
    int v = readReg(REG_INPUT_DIGITAL + channel);
    if (v < 0) return v;
    return (v ? 1 : 0);
}

bool M5Unit8Servos::writePwmDuty(uint8_t channel, uint8_t duty) {
    if (channel > 7) return false;
    if (duty > 100) duty = 100;
    setChannelMode(channel, MODE_PWM);
    return writeReg(REG_PWM_DUTY_BASE + channel, duty);
}
