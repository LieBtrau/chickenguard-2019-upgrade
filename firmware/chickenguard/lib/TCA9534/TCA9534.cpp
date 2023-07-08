#include "TCA9534.h"

TCA9534::TCA9534(bool a0_isHigh, bool a1_isHigh, bool a2_isHigh)
{
    _deviceAddress = 0x20 | (a0_isHigh << 0) | (a1_isHigh << 1) | (a2_isHigh << 2);
}

uint8_t TCA9534::getI2cAddress() const
{
    return _deviceAddress;
}

void TCA9534::attach(int8_t (*readRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data), bool (*writeRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data))
{
    _readRegister = readRegister;
    _writeRegister = writeRegister;
    writePort(0); // default all outputs to low.  This has no real effect until the pins are set to output.
}

bool TCA9534::readPin(const Pins port_pin) const
{
    return readBit(Register::INPUT_PORT, (uint8_t)port_pin);
}
uint8_t TCA9534::readPort() const
{
    return readByte(Register::INPUT_PORT);
}

bool TCA9534::writePin(const Pins port_pin, const bool isHigh) const
{
    return writeBit(Register::OUTPUT_PORT, (uint8_t)port_pin, isHigh);
}

bool TCA9534::writePort(const uint8_t value) const
{
    return writeByte(Register::OUTPUT_PORT, value);
}

bool TCA9534::invertPolarity(const Pins port_pin, bool isInverse) const
{
    return writeBit(Register::POLARITY, (uint8_t)port_pin, isInverse);
}

bool TCA9534::invertAllPolarity(bool isInverse) const
{
    return writeByte(Register::POLARITY, isInverse ? 0xFF : 0x00);
}

/**
 * @brief Set the direction of a pin.
 * configures the directions of the I/O pins
 *      0 = corresponding port pin enabled as an output
 *      1 = corresponding port pin configured as input (default value)
 */
bool TCA9534::setPinDirection(const Pins port_pin, bool isOutput) const
{
    return writeBit(Register::DIRECTION, (uint8_t)port_pin, isOutput ? 0 : 1);
}

/**
 * @brief Set the direction of the pins in the bit mask.
 *      0 = corresponding port pin enabled as an output
 *      1 = corresponding port pin configured as input (default value)
 */
bool TCA9534::setPortDirection(uint8_t mask) const
{
    return writeByte(Register::DIRECTION, mask);
}

/**
 * @brief Read the logic level of a pin.
 * @return true if the pin is high, false if the pin is low.
 */
bool TCA9534::readBit(Register reg, uint8_t bit) const
{
    uint8_t b = readByte(reg);
    b &= (1 << bit);
    return b != 0;
}

uint8_t TCA9534::readByte(Register reg) const
{
    uint8_t data;
    _readRegister(_deviceAddress, (uint8_t)reg, 1, &data);
    return data;
}

bool TCA9534::writeBit(Register reg, uint8_t bit, bool setHigh) const
{
    uint8_t b = readByte(reg);
    b = setHigh ? (b | (1 << bit)) : (b & ~(1 << bit));
    return writeByte(reg, b);
}

bool TCA9534::writeByte(Register reg, uint8_t data) const
{
    return _writeRegister(_deviceAddress, (uint8_t)reg, 1, &data);
}
