#include "i2c_hal.h"
#include <Wire.h>

static TwoWire *_wire = &Wire;

bool i2c_hal_init(int sda, int scl)
{
    _wire->begin(sda,scl);
    return true;
}

bool detectI2cDevice(uint8_t i2c_address)
{
    _wire->beginTransmission(i2c_address);
    return (_wire->endTransmission() == 0);
}

int8_t readBytes(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data)
{
    _wire->beginTransmission(i2c_address);
    _wire->write(reg);
    _wire->endTransmission(false); // restart
    _wire->requestFrom(i2c_address, size);
    int8_t count = 0;
    while (_wire->available())
        data[count++] = _wire->read();
    return count;
}

bool writeBytes(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data)
{
    _wire->beginTransmission(i2c_address);
    _wire->write(reg);
    _wire->write(data, size);
    return (_wire->endTransmission() == 0);
}
