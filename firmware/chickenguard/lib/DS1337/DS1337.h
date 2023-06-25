#pragma once

#include "time.h"
#include "stdbool.h"

class DS1337
{
public:
    DS1337(int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data),
           bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data));
    ~DS1337();
    bool getTime(time_t *t);
    bool getTime(tm *timeinfo);
    bool setTime(time_t time);
    bool setTime(tm *timeinfo);
    uint8_t getI2cAddress();
    bool enableSquareWave(bool isEnabled);
    bool setDailyAlarm1(uint8_t hour, uint8_t minute, uint8_t second = 0);
    bool acknowledgeAlarm1();
    bool isAlarm1Triggered();
    bool isTimeValid();

private:
    // Register addresses
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

    // Register bits
    enum Alarm1SecondsBits
    {
        Alarm1Seconds_A1M1 = 7
    };
    enum Alarm1MinutesBits
    {
        Alarm1Minutes_A1M2 = 7
    };
    enum Alarm1HoursBits
    {
        Alarm1Hours_A1M3 = 7
    };
    enum Alarm1DayDateBits
    {
        Alarm1DayDate_A1M4 = 7
    };
    enum ControlBits
    {
        Control_nETIME = 7,
        Control_INTCN = 2,
        Control_A1IE = 0
    };
    enum StatusBits
    {
        Status_OSF = 7,
        Status_A1F = 0
    };
    uint8_t const I2C_ADDRESS = 0x68;
    int8_t (*_readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data);
    bool (*_writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data);
    bool setRegisterBit(Register reg, uint8_t bit, bool value);
};