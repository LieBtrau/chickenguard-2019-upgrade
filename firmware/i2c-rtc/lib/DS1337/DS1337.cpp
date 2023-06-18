/**
 * @file DS1337.cpp
 * @author Christoph Tack (you@domain.com)
 * @brief Library for DS1337 and PT7C4337 RTC ICs
 * @version 0.1
 * @date 2023-06-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "DS1337.h"
#include "esp32-hal-log.h"
static const char* TAG = "DS1337";
/**
 * @brief Get the Time object as UTC
 * https://stackoverflow.com/questions/530519/stdmktime-and-timezone-info#5157134
 * @param t
 * @return time_t
 */
static time_t timegm(struct tm *t)
{
    long year;
    time_t result;
    const int MONTHSPERYEAR = 12; /* months per calendar year */
    static const int cumdays[MONTHSPERYEAR] =
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    /*@ +matchanyintegral @*/
    year = 1900 + t->tm_year + t->tm_mon / MONTHSPERYEAR;
    result = (year - 1970) * 365 + cumdays[t->tm_mon % MONTHSPERYEAR];
    result += (year - 1968) / 4;
    result -= (year - 1900) / 100;
    result += (year - 1600) / 400;
    if ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0) &&
        (t->tm_mon % MONTHSPERYEAR) < 2)
        result--;
    result += t->tm_mday - 1;
    result *= 24;
    result += t->tm_hour;
    result *= 60;
    result += t->tm_min;
    result *= 60;
    result += t->tm_sec;
    if (t->tm_isdst == 1)
        result -= 3600;
    /*@ -matchanyintegral @*/
    return (result);
}

uint8_t decode_bcd(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

uint8_t encode_bcd(uint8_t hex)
{
    return (((hex / 10) & 0x0F) << 4) + (hex % 10);
}

DS1337::DS1337(int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data), bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data)) : _readBytes(readBytes),
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

bool DS1337::getTime(time_t *t)
{
    tm timeinfo;
    if (!getTime(&timeinfo))
    {
        return false;
    }
    *t = timegm(&timeinfo);
    return true;
}

bool DS1337::getTime(tm *timeinfo)
{
    uint8_t time_data[7];

    if (_readBytes(I2C_ADDRESS, (uint8_t)Register::STATUS, 1, time_data) != 1)
    {
        ESP_LOGE(TAG, "Error reading status register");
        return false;
    }
    if (time_data[0] & (1 << OSF_bit))
    {
        ESP_LOGE(TAG, "Oscillator stopped");
        return false;
    }
    if (_readBytes(I2C_ADDRESS, (uint8_t)Register::SECONDS, sizeof(time_data), time_data) != sizeof(time_data))
    {
        ESP_LOGE(TAG, "Error reading time");
        return false;
    }
    timeinfo->tm_sec = decode_bcd(time_data[0]);
    timeinfo->tm_min = decode_bcd(time_data[1]);
    timeinfo->tm_hour = decode_bcd(time_data[2]);
    timeinfo->tm_wday = decode_bcd(time_data[3]);
    timeinfo->tm_mday = decode_bcd(time_data[4]);
    timeinfo->tm_mon = decode_bcd(time_data[5]);
    // 21st century, so adding 100 to the year
    timeinfo->tm_year = decode_bcd(time_data[6])+100;
    ESP_LOGI(TAG, "The current UTC date/time is: %s", asctime(timeinfo));
    return true;
}

bool DS1337::setTime(time_t time)
{
    tm *timeinfo = gmtime(&time);
    return setTime(timeinfo);
}

bool DS1337::setTime(tm *timeinfo)
{
    uint8_t time_data[7];

    setRegisterBit(Register::CONTROL, nETIME_bit, true);
    // Write the time
    time_data[0] = encode_bcd(timeinfo->tm_sec);
    time_data[1] = encode_bcd(timeinfo->tm_min);
    time_data[2] = encode_bcd(timeinfo->tm_hour);
    time_data[3] = encode_bcd(timeinfo->tm_wday);
    time_data[4] = encode_bcd(timeinfo->tm_mday);
    time_data[5] = encode_bcd(timeinfo->tm_mon);
    // timeinfo->tm_year is years since 1900, needs to be converted to 0-99
    time_data[6] = encode_bcd(timeinfo->tm_year % 100);
    if (!_writeBytes(I2C_ADDRESS, (uint8_t)Register::SECONDS, sizeof(time_data), time_data))
    {
        ESP_LOGE(TAG, "Error writing time");
        return false;
    }
    // Start the clock
    setRegisterBit(Register::CONTROL, nETIME_bit, false);
    // Clear the oscillator stop flag, indicating that the time is valid again.
    setRegisterBit(Register::STATUS, OSF_bit, false);
    return true;
}

bool DS1337::setDailyAlarm1(uint8_t hour, uint8_t minute, uint8_t second)
{
    uint8_t alarm_data[3];

    alarm_data[0] = encode_bcd(second);
    alarm_data[1] = encode_bcd(minute);
    alarm_data[2] = encode_bcd(hour);
    if (!_writeBytes(I2C_ADDRESS, (uint8_t)Register::ALARM1_SECONDS, sizeof(alarm_data), alarm_data))
    {
        ESP_LOGE(TAG, "Error writing alarm");
        return false;
    }
    // Set alarm frequency to once per day
    setRegisterBit(Register::ALARM1_SECONDS, A1M1_bit, false);
    setRegisterBit(Register::ALARM1_MINUTES, A1M2_bit, false);
    setRegisterBit(Register::ALARM1_HOURS, A1M3_bit, false);
    setRegisterBit(Register::ALARM1_DAY_DATE, A1M4_bit, true);
    // Enable alarm
    setRegisterBit(Register::CONTROL, A1IE_bit, true);
    return true;
}

bool DS1337::setRegisterBit(Register reg, uint8_t bit, bool value)
{
    uint8_t reg_data;
    if (_readBytes(I2C_ADDRESS, (uint8_t)reg, sizeof(reg_data), &reg_data) != sizeof(reg_data))
    {
        ESP_LOGE(TAG, "Error reading register");
        return false;
    }
    if (value)
        reg_data |= (1 << bit);
    else
        reg_data &= ~(1 << bit);
    return _writeBytes(I2C_ADDRESS, (uint8_t)reg, sizeof(reg_data), &reg_data);
}

bool DS1337::enableSquareWave(bool isEnabled)
{
    return setRegisterBit(Register::CONTROL, INTCN_bit, !isEnabled);
}

bool DS1337::acknowledgeAlarm1()
{
    return setRegisterBit(Register::STATUS, A1F_bit, false);
}

bool DS1337::isAlarm1Triggered()
{
    uint8_t status_data;
    if (_readBytes(I2C_ADDRESS, (uint8_t)Register::STATUS, sizeof(status_data), &status_data) != sizeof(status_data))
    {
        ESP_LOGE(TAG, "Error reading status register");
        return false;
    }
    return status_data & (1 << A1F_bit);
}