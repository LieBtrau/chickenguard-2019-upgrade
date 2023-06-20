#include "timeControl.h"

#include <SolarCalculator.h>

static const char *TAG = "timeControl";

typedef struct
{
    char name[20];
    char tz[30];
} timeZone_t;

static const timeZone_t timeZones[1] =
    {
        {"Europe/Brussels", "CET-1CEST,M3.5.0,M10.5.0/3"}}; // source : last line of /usr/share/zoneinfo/Europe/Brussels)

TimeControl::TimeControl()
{
    _timeIsSet = false;
    _timeZoneIndex = 0;
}

TimeControl::~TimeControl()
{
}

bool TimeControl::updateMcuTime(long utc, const String timeZone)
{
    ESP_LOGI(TAG, "%lu", utc);
    timeval epoch = {utc, 0};
    settimeofday((const timeval *)&epoch, 0);
    ESP_LOGI(TAG, "%s", timeZone.c_str());

    for (int i = 0; i < sizeof(timeZones) / sizeof(timeZone_t); i++)
    {
        if (timeZone == timeZones[i].name)
        {
            _timeIsSet = true;
            _timeZoneIndex = i;
            setenv("TZ", timeZones[i].tz, 1);
            tzset();
        }
        return true;
    }
    // no matching timezone found
    return false;
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
