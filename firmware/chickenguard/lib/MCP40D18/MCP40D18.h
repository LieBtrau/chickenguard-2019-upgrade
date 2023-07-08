#pragma once
#include <stdint.h>

class MCP40D18
{
public:
    MCP40D18(){};
    void attach(int8_t (*readRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data),
                bool (*writeRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data));
    uint8_t getI2cAddress() const;
    void setWiper(uint8_t value) const;

private:
    int8_t (*_readRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data) = nullptr;
    bool (*_writeRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data) = nullptr;
    /**
     * I2C address of device.
     */
    const uint8_t _deviceAddress = 0x2E;
    const uint8_t REG_CMD = 0x00;
};