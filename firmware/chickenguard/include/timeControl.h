#pragma once

#include <Arduino.h>
#include "DS1337.h"
#include "AsyncDelay.h"

class TimeControl
{
public:
    TimeControl(int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data),
                bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data));
    ~TimeControl();

    bool init(String timeZone);
    bool hasValidTime();
    bool updateMcuTime(long utc, const String timeZone);
    bool setOpenAlarmSunrise(double latitude, double longitude);
    bool setOpenAlarmFixTime(uint8_t hour, uint8_t minute);
    bool setCloseAlarmSunset(double latitude, double longitude);
    bool setCloseAlarmFixTime(uint8_t hour, uint8_t minute);
    bool disableAlarms();
    bool openDoorAlarmTriggered();
    bool closeDoorAlarmTriggered();
private:
    bool setTimeZone(String timeZone);
    void doubleToHrMin(double time, uint8_t *hr, uint8_t *min);
    struct tm* localToUtcTimeObject(uint8_t hourLocal, uint8_t minuteLocal);
    struct tm *utcToUtcTimeObject(uint8_t hourUtc, uint8_t minuteUtc);
    void printLocalTime();
    DS1337 _rtc;
    bool _timeZoneSet = false;
};