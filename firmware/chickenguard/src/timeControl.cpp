/**
 * @file timeControl.cpp
 * @author Christoph Tack
 * @brief
 * @version 0.1
 * @date 2023-06-25
 * @details
 *   When using the sunrise and sunset functions, the new sunset alarm will be set just after sunrise.  The new sunrise alarm will be set
 * just after sunset.  This is to prevent the alarm from triggering immediately after setting it (which would be the case for sunrise times
 * after summer solstice and sunset times before summer solstice).
 * @copyright Copyright (c) 2023
 *
 */
#include "timeControl.h"
#include "i2c_hal.h"
#include <SolarCalculator.h>

static const char *TAG = "timeControl";

static time_t timegm(struct tm *t);

typedef struct
{
    char name[20];
    char tz[30];
} timeZone_t;

static const timeZone_t timeZones[1] =
    {
        {"Europe/Brussels", "CET-1CEST,M3.5.0,M10.5.0/3"} // source : last line of /usr/share/zoneinfo/Europe/Brussels)
                                                          // Add more timezones here
};

TimeControl::TimeControl(int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data),
                         bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data)) : 
                         _rtc(readBytes, writeBytes), 
                         _timeZoneSet(false)
{
}

TimeControl::~TimeControl()
{
}

/**
 * @brief Initialize the time control
 *
 * @param timeZone
 * @return true when RTC could be initialized
 * @return false when RTC could not be initialized
 */
bool TimeControl::init(String timeZone)
{
    assert(detectI2cDevice(_rtc.getI2cAddress()));
    if (_rtc.isTimeValid())
    {
        struct tm timeinfo;
        assert(_rtc.getTime(&timeinfo));
        time_t now = timegm(&timeinfo);
        // set current time to MCU
        timeval epoch = {now, 0};
        settimeofday((const timeval *)&epoch, 0);

        // print UTC-time using strftime() instead of the obsolete asctime()
        gmtime_r(&now, &timeinfo);
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current UTC date/time is: %s", strftime_buf);

        // set timezone
        if (setTimeZone(timeZone))
        {
            _timeZoneSet = true;
            // Print local time
            time(&now);
            localtime_r(&now, &timeinfo);
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "The current local date/time is: %s", strftime_buf);
        }
    }
    return _rtc.enableSquareWave(false); // disable square wave output to save power
}

bool TimeControl::openDoorAlarmTriggered()
{
    if (_rtc.isAlarmTriggered(DS1337::AlarmType::Alarm1))
    {
        ESP_LOGI(TAG, "Alarm 1 triggered");
        _rtc.acknowledgeAlarm(DS1337::AlarmType::Alarm1);
        return true;
    }
    return false;
}

bool TimeControl::closeDoorAlarmTriggered()
{
    if (_rtc.isAlarmTriggered(DS1337::AlarmType::Alarm2))
    {
        ESP_LOGI(TAG, "Alarm 2 triggered");
        _rtc.acknowledgeAlarm(DS1337::AlarmType::Alarm2);
        return true;
    }
    return false;
}

bool TimeControl::hasValidTime()
{
    return _rtc.isTimeValid() && _timeZoneSet;
}

bool TimeControl::updateMcuTime(long utc, const String timeZone)
{
    ESP_LOGI(TAG, "UTC : %lu", utc);
    timeval epoch = {utc, 0};
    settimeofday((const timeval *)&epoch, 0);
    ESP_LOGI(TAG, "TimeZone : %s", timeZone.c_str());

    tm *timeinfo = gmtime(&utc);
    if (!_rtc.setTime(timeinfo))
    {
        ESP_LOGE(TAG, "Could not set RTC time");
        return false;
    }
    return setTimeZone(timeZone);
}

/**
 * @brief Set the time zone
 *
 * @param timeZone
 * @return true when time zone is set
 * @return false when time zone is not in the list of time zones
 */
bool TimeControl::setTimeZone(String timeZone)
{
    for (int i = 0; i < sizeof(timeZones) / sizeof(timeZone_t); i++)
    {
        if (timeZone == timeZones[i].name)
        {
            setenv("TZ", timeZones[i].tz, 1);
            tzset();
            _timeZoneSet = true;
            break;
        }
    }
    return _timeZoneSet;
}

bool TimeControl::setOpenAlarmSunrise(double latitude, double longitude)
{
    if (!hasValidTime())
    {
        ESP_LOGE(TAG, "Time is not set");
        return false;
    }
    time_t utc;
    time(&utc);
    double transit, sunrise, sunset;
    calcSunriseSunset(utc, latitude, longitude, transit, sunrise, sunset);
    uint8_t hour, minute;
    doubleToHrMin(sunrise, &hour, &minute);
    ESP_LOGI(TAG, "Next Sunrise alarm (local time): %02d:%02d", hour, minute);
    return _rtc.setDailyAlarm(DS1337::AlarmType::Alarm1, toUtc(hour, minute));
}

bool TimeControl::setCloseAlarmSunset(double latitude, double longitude)
{
    if (!hasValidTime())
    {
        ESP_LOGE(TAG, "Time is not set");
        return false;
    }
    time_t utc;
    time(&utc);
    double transit, sunrise, sunset;
    calcSunriseSunset(utc, latitude, longitude, transit, sunrise, sunset);
    uint8_t hour, minute;
    doubleToHrMin(sunset, &hour, &minute);
    ESP_LOGI(TAG, "Next Sunset alarm (local time): %02d:%02d", hour, minute);
    return _rtc.setDailyAlarm(DS1337::AlarmType::Alarm2, toUtc(hour, minute));
}

bool TimeControl::setOpenAlarmFixTime(uint8_t hour, uint8_t minute)
{
    if (!hasValidTime())
    {
        ESP_LOGE(TAG, "Time is not set");
        return false;
    }
    ESP_LOGI(TAG, "Next fix open alarm (local time): %02d:%02d", hour, minute);
    return _rtc.setDailyAlarm(DS1337::AlarmType::Alarm1, toUtc(hour, minute));
}

bool TimeControl::setCloseAlarmFixTime(uint8_t hour, uint8_t minute)
{
    if (!hasValidTime())
    {
        ESP_LOGE(TAG, "Time is not set");
        return false;
    }
    ESP_LOGI(TAG, "Next fix close alarm (local time): %02d:%02d", hour, minute);
    return _rtc.setDailyAlarm(DS1337::AlarmType::Alarm2, toUtc(hour, minute));
}

void TimeControl::doubleToHrMin(double time, uint8_t *hour, uint8_t *minute)
{
    int m = int(round(time * 60));
    *hour = (m / 60) % 24;
    *minute = m % 60;
}

bool TimeControl::disableAlarms()
{
    return _rtc.disableAlarm(DS1337::AlarmType::Alarm1) && _rtc.disableAlarm(DS1337::AlarmType::Alarm2);
}

/**
 * @brief Convert hour and minute to UTC
 * Useful for setting the alarm time, because the RTC is in UTC
 * @param hour
 * @param minute
 * @return struct tm*
 */
struct tm *TimeControl::toUtc(uint8_t hour, uint8_t minute)
{
    time_t dummyTime;
    time(&dummyTime);
    struct tm *timeinfo = localtime(&dummyTime);
    timeinfo->tm_hour = hour;
    timeinfo->tm_min = minute;
    timeinfo->tm_sec = 0;
    time_t utc = mktime(timeinfo);
    gmtime_r(&utc, timeinfo);
    return timeinfo;
}

/**
 * @brief Get the Time object as UTC
 * https://stackoverflow.com/questions/530519/stdmktime-and-timezone-info#5157134
 * @param t
 * @return time_t
 */
time_t timegm(struct tm *t)
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