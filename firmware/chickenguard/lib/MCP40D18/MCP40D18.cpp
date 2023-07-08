#include "MCP40D18.h"

uint8_t MCP40D18::getI2cAddress() const
{
    return _deviceAddress;
}

void MCP40D18::attach(int8_t (*readRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data), bool (*writeRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data))
{
    _readRegister = readRegister;
    _writeRegister = writeRegister;
    setWiper(0);
}

void MCP40D18::setWiper(uint8_t value) const
{
    _writeRegister(_deviceAddress, REG_CMD, 1, &value);
}
