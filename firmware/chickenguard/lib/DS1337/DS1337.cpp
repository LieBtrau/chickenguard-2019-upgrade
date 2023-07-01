/**
 * @file DS1337.cpp
 * @author Christoph Tack (you@domain.com)
 * @brief Library for DS1337 and PT7C4337 RTC ICs
 * @version 0.1
 * @date 2023-06-18
 * @note  This library is timezone agnostic.  It is up to the user to convert to and from UTC.  
 *       I suggest using UTC time for the RTC, so that the RTC can be used in different timezones and doesn't need to be
 *      updated when the timezone or daylight savings time changes.
 *
 * @details
 * * [PT7C4337](https://www.diodes.com/assets/Datasheets/PT7C4337.pdf)
 * * [DS1337](https://www.analog.com/media/en/technical-documentation/data-sheets/DS1337-DS1337C.pdf)
 * @copyright Copyright (c) 2023
 *
 */

#include "DS1337.h"
#include "esp32-hal-log.h"
#include "../../include/bit_manipulation.h"
static const char *TAG = "DS1337";

uint8_t decode_bcd(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

uint8_t encode_bcd(uint8_t hex)
{
    return (((hex / 10) & 0x0F) << 4) + (hex % 10);
}

DS1337::DS1337(
    int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data),
    bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data)) : _readBytes(readBytes),
                                                                                               _writeBytes(writeBytes)
{
}

DS1337::~DS1337()
{
}

uint8_t DS1337::getI2cAddress()
{
    return I2C_ADDRESS;
}

bool DS1337::getTime(tm *timeinfo)
{
    uint8_t time_data[7];

    if (!isTimeValid())
    {
        ESP_LOGE(TAG, "Time is not valid");
        return false;
    }
    assert(_readBytes(I2C_ADDRESS, (uint8_t)Register::SECONDS, sizeof(time_data), time_data) == sizeof(time_data));
    timeinfo->tm_sec = decode_bcd(time_data[0]);
    timeinfo->tm_min = decode_bcd(time_data[1]);
    timeinfo->tm_hour = decode_bcd(time_data[2]);
    timeinfo->tm_wday = decode_bcd(time_data[3]);
    timeinfo->tm_mday = decode_bcd(time_data[4]);
    timeinfo->tm_mon = decode_bcd(time_data[5]);
    // 21st century, so adding 100 to the year
    timeinfo->tm_year = decode_bcd(time_data[6]) + 100;
    ESP_LOGI(TAG, "The current RTC date/time is: %s", asctime(timeinfo));
    return true;
}


bool DS1337::setTime(tm *timeinfo)
{
    uint8_t time_data[7];

    setRegisterBit(Register::CONTROL, Control_nETIME, true);
    // Write the time
    time_data[0] = encode_bcd(timeinfo->tm_sec);
    time_data[1] = encode_bcd(timeinfo->tm_min);
    time_data[2] = encode_bcd(timeinfo->tm_hour);
    time_data[3] = encode_bcd(timeinfo->tm_wday);
    time_data[4] = encode_bcd(timeinfo->tm_mday);
    time_data[5] = encode_bcd(timeinfo->tm_mon);
    // timeinfo->tm_year is years since 1900, needs to be converted to 0-99
    time_data[6] = encode_bcd(timeinfo->tm_year % 100);
    assert(_writeBytes(I2C_ADDRESS, (uint8_t)Register::SECONDS, sizeof(time_data), time_data));

    return (setRegisterBit(Register::CONTROL, Control_nETIME, false) && // Start the clock
            setRegisterBit(Register::STATUS, Status_OSF, false));       // Clear the oscillator stop flag, indicating that the time is valid again.
}

bool DS1337::setDailyAlarm(AlarmType alarm, tm *timeinfo)
{
    uint8_t alarm_data[2];

    ESP_LOGI(TAG, "Setting RTC alarm %d to %02d:%02d", alarm, timeinfo->tm_hour, timeinfo->tm_min);

    alarm_data[0] = encode_bcd(timeinfo->tm_min);
    alarm_data[1] = encode_bcd(timeinfo->tm_hour);

    switch (alarm)
    {
    case AlarmType::Alarm1:
        assert(_writeBytes(I2C_ADDRESS, (uint8_t)Register::ALARM1_MINUTES, sizeof(alarm_data), alarm_data));
        // Set alarm frequency to once per day
        assert(setRegisterBit(Register::ALARM1_SECONDS, Alarm1Seconds_A1M1, false));
        assert(setRegisterBit(Register::ALARM1_MINUTES, Alarm1Minutes_A1M2, false));
        assert(setRegisterBit(Register::ALARM1_HOURS, Alarm1Hours_A1M3, false));
        assert(setRegisterBit(Register::ALARM1_DAY_DATE, Alarm1DayDate_A1M4, true));
        // Enable alarm
        assert(setRegisterBit(Register::CONTROL, Control_A1IE, true));
        break;
    case AlarmType::Alarm2:
        assert(_writeBytes(I2C_ADDRESS, (uint8_t)Register::ALARM2_MINUTES, sizeof(alarm_data), alarm_data));
        // Set alarm frequency to once per day
        assert(setRegisterBit(Register::ALARM2_MINUTES, Alarm2Minutes_A2M2, false));
        assert(setRegisterBit(Register::ALARM2_HOURS, Alarm2Hours_A2M3, false));
        assert(setRegisterBit(Register::ALARM2_DAY_DATE, Alarm2DayDate_A2M4, true));
        // Enable alarm
        assert(setRegisterBit(Register::CONTROL, Control_A2IE, true));
        break;
    default:
        assert(false);
    }
    return true;
}

bool DS1337::setRegisterBit(Register reg, uint8_t bit, bool value)
{
    uint8_t reg_data;
    assert(_readBytes(I2C_ADDRESS, (uint8_t)reg, sizeof(reg_data), &reg_data) == sizeof(reg_data));
    bitWrite(reg_data, bit, value);
    return _writeBytes(I2C_ADDRESS, (uint8_t)reg, sizeof(reg_data), &reg_data);
}

bool DS1337::enableSquareWave(bool isEnabled)
{
    return setRegisterBit(Register::CONTROL, Control_INTCN, !isEnabled);
}

bool DS1337::acknowledgeAlarm(AlarmType alarm)
{
    switch (alarm)
    {
    case AlarmType::Alarm1:
        return setRegisterBit(Register::STATUS, Status_A1F, false);
    case AlarmType::Alarm2:
        return setRegisterBit(Register::STATUS, Status_A2F, false);
    default:
        assert(false);
    }
}

bool DS1337::isAlarmTriggered(AlarmType alarm)
{
    uint8_t status;
    assert(readStatusRegister(&status));
    switch (alarm)
    {
    case AlarmType::Alarm1:
        return bitRead(status, Status_A1F);
    case AlarmType::Alarm2:
        return bitRead(status, Status_A2F);
    default:
        assert(false);
    }
}

bool DS1337::isTimeValid()
{
    uint8_t status;
    assert(readStatusRegister(&status));
    return !bitRead(status, Status_OSF);
}

bool DS1337::readStatusRegister(uint8_t *status)
{
    return _readBytes(I2C_ADDRESS, (uint8_t)Register::STATUS, sizeof(*status), status) == sizeof(*status);
}

bool DS1337::disableAlarm(AlarmType alarm)
{
    switch (alarm)
    {
    case AlarmType::Alarm1:
        return setRegisterBit(Register::CONTROL, Control_A1IE, false);
    case AlarmType::Alarm2:
        return setRegisterBit(Register::CONTROL, Control_A2IE, false);
    default:
        return false;
    }
}