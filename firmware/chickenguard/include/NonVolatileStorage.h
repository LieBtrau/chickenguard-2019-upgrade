#pragma once
#include "Preferences.h"

class NonVolatileStorage
{
public:
    enum class DoorControl
    {
        Manual,
        FixTime,
        SunriseSunset
    };

    NonVolatileStorage();
    ~NonVolatileStorage();
    void restoreAll();
    void saveAll();
    void getGeoLocation(float& latitude, float& longitude) const;
    void setGeoLocation(float latitude, float longitude);
    void getFixOpeningTime(uint8_t hour, uint8_t minutes) const;
    void setFixOpeningTime(String hour_minutes);
    void getFixClosingTime(uint8_t hour, uint8_t minutes) const;
    void setFixClosingTime(String hour_minutes);
    DoorControl getDoorControl() const;
    void setDoorControl(String doorControl);

private:
    bool parseTimeString(String hour_minutes, uint8_t& hour, uint8_t& minutes);
    Preferences _preferences;
    float _latitude = 0;
    float _longitude = 0;
    uint8_t _fixOpeningTime_hour = 0;
    uint8_t _fixOpeningTime_minute = 0;
    uint8_t _fixClosingTime_hour = 0;
    uint8_t _fixClosingTime_minute = 0;
    DoorControl _doorControl = DoorControl::Manual;
};