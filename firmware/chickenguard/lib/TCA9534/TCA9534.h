/**
 * @file TCA9534.h
 * [Datasheet](https://www.ti.com/lit/ds/symlink/tca9534.pdf)
 */
#pragma once
#include <stdint.h>
#include "../LCD_I2C/LiquidCrystal_I2C.h"


class TCA9534 : public IOexpander
{
public:
    enum class Pins
    {
        P0= 0,
        P1= 1,
        P2= 2,
        P3= 3,
        P4= 4,
        P5= 5,
        P6= 6,
        P7= 7
    };
    TCA9534(bool a0_isHigh = false, bool a1_isHigh = false, bool a2_isHigh = false);
    uint8_t getI2cAddress() const;
    void attach(int8_t (*readRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data), bool (*writeRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data));
    bool readPin(const Pins port_pin) const;
    uint8_t readPort() const;
    bool writePin(const Pins port_pin, const bool isHigh = true) const;
    bool writePort(const uint8_t value) const override;
    bool invertPolarity(const Pins port_pin, bool isInverse = true) const;
    bool invertAllPolarity(bool isInverse = true) const;
    bool setPinDirection(const Pins port_pin, bool isOutput = true) const;
    bool setPortDirection(uint8_t mask) const;

private:
    enum class Register
    {
        INPUT_PORT,
        OUTPUT_PORT,
        POLARITY,
        DIRECTION
    };

    bool readBit(Register reg, uint8_t bit) const;
    uint8_t readByte(Register reg) const;
    bool writeBit(Register reg, uint8_t bit, bool setHigh) const;
    bool writeByte(Register reg, uint8_t data) const;

    int8_t (*_readRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data) = nullptr;
    bool (*_writeRegister)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data) = nullptr;
    /**
     * I2C address of device.
     */
    uint8_t _deviceAddress;
};