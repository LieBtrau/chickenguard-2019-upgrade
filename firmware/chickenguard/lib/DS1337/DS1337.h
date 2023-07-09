#pragma once

#include "time.h"
#include "stdbool.h"

class DS1337
{
public:
    enum class AlarmType
    {
        Alarm1,
        Alarm2
    };
    DS1337(int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data),
           bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data));
    ~DS1337();
    bool isTimeValid();
    bool getTime(tm *timeinfo);
    bool setTime(tm *timeinfo);
    uint8_t getI2cAddress();
    bool enableSquareWave(bool isEnabled);
    bool setDailyAlarm(AlarmType alarm, tm *timeinfo);
    bool isAlarmTriggered(AlarmType alarm);
    bool acknowledgeAlarm(AlarmType alarm);
    bool disableAlarm(AlarmType alarm);
private:
    // Register addresses
    enum class Register
    {
        SECONDS = 0x00,
        ALARM1_SECONDS = 0x07,
        ALARM1_MINUTES = 0x08,
        ALARM1_HOURS = 0x09,
        ALARM1_DAY_DATE = 0x0A,
        ALARM2_MINUTES = 0x0B,
        ALARM2_HOURS = 0x0C,
        ALARM2_DAY_DATE = 0x0D,
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
    enum Alarm2MinutesBits
    {
        Alarm2Minutes_A2M2 = 7
    };
    enum Alarm2HoursBits
    {
        Alarm2Hours_A2M3 = 7
    };
    enum Alarm2DayDateBits
    {
        Alarm2DayDate_A2M4 = 7
    };
    enum ControlBits
    {
        Control_nETIME = 7, //!< Enable oscillator
        Control_INTCN = 2,  //!< Interrupt control
        Control_RS2 = 4,    //!< Rate select
        Control_RS1 = 3,    //!< Rate select
        Control_A2IE = 1,   //!< Alarm 2 interrupt enable
        Control_A1IE = 0    //!< Alarm 1 interrupt enable
    };
    enum StatusBits
    {
        Status_OSF = 7, //!< Oscillator stop flag
        Status_A2F = 1, //!< Alarm 2 flag
        Status_A1F = 0  //!< Alarm 1 flag
    };
    uint8_t const I2C_ADDRESS = 0x68;
    int8_t (*_readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data);
    bool (*_writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data);
    bool setRegisterBit(Register reg, uint8_t bit, bool value);
    bool readStatusRegister(uint8_t *status);
};