#pragma once

#include <Arduino.h>
#include "DS1337.h"


class TimeControl
{
public:
    TimeControl(int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data), 
        bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data));
    ~TimeControl();

    bool init(String timeZone);
    bool hasValidTime();
    bool updateMcuTime(long utc, const String timeZone);

private:
    bool setTimeZone(String timeZone);
    DS1337 _rtc;
    bool _timeZoneSet=false;
};