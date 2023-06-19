#pragma once

#include "time.h"
#include "stdbool.h"

class DS1337
{
public:
    DS1337(int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data), bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data));
    ~DS1337();
    bool getTime(time_t *t);
    bool getTime(tm *timeinfo);
    bool setTime(time_t time);
    bool setTime(tm *timeinfo);
    uint8_t getI2cAddress();
    bool enableSquareWave(bool isEnabled);
    bool setDailyAlarm1(uint8_t hour, uint8_t minute, uint8_t second=0);
    bool acknowledgeAlarm1();
    bool isAlarm1Triggered();
private:
    enum class Register
    {
        SECONDS = 0x00,
        ALARM1_SECONDS = 0x07,
        ALARM1_MINUTES = 0x08,
        ALARM1_HOURS = 0x09,
        ALARM1_DAY_DATE = 0x0A,
        CONTROL = 0x0E,
        STATUS = 0x0F
    };
    // Alarm1 seconds register bits
    static const uint8_t A1M1_bit = 7;
    // Alarm1 minutes register bits
    static const uint8_t A1M2_bit = 7;
    // Alarm1 hours register bits
    static const uint8_t A1M3_bit = 7;
    // Alarm1 day/date register bits
    static const uint8_t A1M4_bit = 7;
    // Control register bits
    static const uint8_t nETIME_bit = 7;
    static const uint8_t INTCN_bit = 2;
    static const uint8_t A1IE_bit = 0;
    // Status register bits
    static const uint8_t OSF_bit = 7;
    static const uint8_t A1F_bit = 0;

    uint8_t const I2C_ADDRESS = 0x68;
    int8_t (*_readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data);
    bool (*_writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data);
    bool setRegisterBit(Register reg, uint8_t bit, bool value);
};