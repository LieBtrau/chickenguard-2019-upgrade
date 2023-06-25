#include "timeControl.h"
#include "i2c_hal.h"
#include <SolarCalculator.h>

static const char *TAG = "timeControl";

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

TimeControl::TimeControl(
    int8_t (*readBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data),
    bool (*writeBytes)(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data)) : _rtc(readBytes, writeBytes),
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
        time_t now;
        // get current time from RTC
        assert(_rtc.getTime(&now));
        // set current time to MCU
        timeval epoch = {now, 0};
        settimeofday((const timeval *)&epoch, 0);

        // print UTC-time using strftime() instead of the obsolete asctime()
        struct tm timeinfo;
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

    if (!_rtc.setTime(utc))
    {
        ESP_LOGE(TAG, "Could not set RTC time");
        return false;
    }
    return setTimeZone(timeZone);
}

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
// bool TimeControl::getSunrise(uint8_t *hour, uint8_t *minute, float latitude, float longitude)
// {
//     if (!_timeIsSet)
//     {
//         ESP_LOGE(TAG, "Time is not set");
//         return false;
//     }
//     time_t utc;
//     time(&utc);
//     double transit, sunrise, sunset;
//     calcSunriseSunset(utc, latitude, longitude, transit, sunrise, sunset);
//     ESP_LOGI(TAG, "Sunrise: %f\tSunset: %f\r\n", sunrise, sunset);
//     return true;
// }
