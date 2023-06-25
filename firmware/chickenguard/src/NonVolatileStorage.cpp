#include "NonVolatileStorage.h"
#include "esp_log.h"

static const char *TAG = "NonVolatileStorage";

const bool RO_MODE = true;
const bool RW_MODE = false;
const char *NVS_NAMESPACE = "door";
const char *NVS_KEY_INIT = "nvsInit";
const char *NVS_KEY_LATITUDE = "latitude";
const char *NVS_KEY_LONGITUDE = "longitude";
const char *NVS_KEY_FIX_OPENING_TIME_HOUR = "open_hour";
const char *NVS_KEY_FIX_OPENING_TIME_MINUTE = "open_minute";
const char *NVS_KEY_FIX_CLOSING_TIME_HOUR = "close_hour";
const char *NVS_KEY_FIX_CLOSING_TIME_MINUTE = "close_minute";
const char *NVS_KEY_DOOR_CONTROL = "doorControl";
const char *NVS_KEY_TIME_ZONE = "timeZone";

NonVolatileStorage::NonVolatileStorage()
{
}

NonVolatileStorage::~NonVolatileStorage()
{
}

void NonVolatileStorage::restoreAll()
{
    _preferences.begin(NVS_NAMESPACE, RO_MODE);
    if (!_preferences.isKey(NVS_KEY_INIT))
    {
        _preferences.end();
        ESP_LOGI(TAG, "Initializing NVS for the first time");
        saveAll();
    }
    else
    {
        _latitude = getFloat(NVS_KEY_LATITUDE, 0);
        _longitude = getFloat(NVS_KEY_LONGITUDE, 0);
        _fixOpeningTime_hour = getUChar(NVS_KEY_FIX_OPENING_TIME_HOUR, 0);
        _fixOpeningTime_minute = getUChar(NVS_KEY_FIX_OPENING_TIME_MINUTE, 0);
        _fixClosingTime_hour = getUChar(NVS_KEY_FIX_CLOSING_TIME_HOUR, 0);
        _fixClosingTime_minute = getUChar(NVS_KEY_FIX_CLOSING_TIME_MINUTE, 0);
        _doorControl = static_cast<DoorControl>(getUChar(NVS_KEY_DOOR_CONTROL, 0));
        _timeZone = getString(NVS_KEY_TIME_ZONE, "");
        _preferences.end();
    }
}

float NonVolatileStorage::getFloat(const char *key, float defaultValue)
{
    if (!_preferences.isKey(key))
    {
        return defaultValue;
    }
    return _preferences.getFloat(key, defaultValue);
}
uint8_t NonVolatileStorage::getUChar(const char *key, uint8_t defaultValue)
{
    if (!_preferences.isKey(key))
    {
        return defaultValue;
    }
    return _preferences.getUChar(key, defaultValue);
}

String NonVolatileStorage::getString(const char *key, String defaultValue)
{
    if (!_preferences.isKey(key))
    {
        return defaultValue;
    }
    return _preferences.getString(key, defaultValue);
}

void NonVolatileStorage::saveAll()
{
    ESP_LOGI(TAG, "Saving all settings to NVS");
    _preferences.begin(NVS_NAMESPACE, RW_MODE);
    _preferences.putFloat(NVS_KEY_LATITUDE, _latitude);
    _preferences.putFloat(NVS_KEY_LONGITUDE, _longitude);
    _preferences.putUChar(NVS_KEY_FIX_OPENING_TIME_HOUR, _fixOpeningTime_hour);
    _preferences.putUChar(NVS_KEY_FIX_OPENING_TIME_MINUTE, _fixOpeningTime_minute);
    _preferences.putUChar(NVS_KEY_FIX_CLOSING_TIME_HOUR, _fixClosingTime_hour);
    _preferences.putUChar(NVS_KEY_FIX_CLOSING_TIME_MINUTE, _fixClosingTime_minute);
    _preferences.putUChar(NVS_KEY_DOOR_CONTROL, static_cast<uint8_t>(_doorControl));
    _preferences.putString(NVS_KEY_TIME_ZONE, _timeZone);

    _preferences.putBool(NVS_KEY_INIT, true);
    _preferences.end();
}

void NonVolatileStorage::setGeoLocation(float latitude, float longitude)
{
    if (latitude < -90 || latitude > 90)
    {
        ESP_LOGE(TAG, "Latitude out of range: %2f", latitude);
        return;
    }
    if (longitude < -180 || longitude > 180)
    {
        ESP_LOGE(TAG, "Longitude out of range: %2f", longitude);
        return;
    }
    latitude = _latitude;
    longitude = _longitude;
}

void NonVolatileStorage::getGeoLocation(float &latitude, float &longitude) const
{
    latitude = _latitude;
    longitude = _longitude;
}

void NonVolatileStorage::getFixOpeningTime(uint8_t hour, uint8_t minutes) const
{
    hour = _fixOpeningTime_hour;
    minutes = _fixOpeningTime_minute;
}

void NonVolatileStorage::setFixOpeningTime(String hour_minutes)
{
    uint8_t hour = 0;
    uint8_t minutes = 0;
    if (parseTimeString(hour_minutes, hour, minutes))
    {
        _fixOpeningTime_hour = hour;
        _fixOpeningTime_minute = minutes;
    }
}

void NonVolatileStorage::getFixClosingTime(uint8_t hour, uint8_t minutes) const
{
    hour = _fixClosingTime_hour;
    minutes = _fixClosingTime_minute;
}
void NonVolatileStorage::setFixClosingTime(String hour_minutes)
{
    uint8_t hour = 0;
    uint8_t minutes = 0;
    if (parseTimeString(hour_minutes, hour, minutes))
    {
        _fixClosingTime_hour = hour;
        _fixClosingTime_minute = minutes;
    }
}

NonVolatileStorage::DoorControl NonVolatileStorage::getDoorControl() const { return _doorControl; }

/**
 * @brief Set the Door Control object
 * The fixed strings are the same as defined in index.js
 * @param doorControl
 */
void NonVolatileStorage::setDoorControl(String doorControl)
{
    if (doorControl.equals("manual"))
    {
        _doorControl = DoorControl::Manual;
    }
    else if (doorControl.equals("fixedTime"))
    {
        _doorControl = DoorControl::FixTime;
    }
    else if (doorControl.equals("sun"))
    {
        _doorControl = DoorControl::SunriseSunset;
    }
    else
    {
        ESP_LOGE(TAG, "Invalid door control: %s", doorControl.c_str());
        return;
    }
}

void NonVolatileStorage::setTimeZone(String timeZone)
{
    _timeZone = timeZone;
}

String NonVolatileStorage::getTimeZone() const
{
    return _timeZone;
}

bool NonVolatileStorage::parseTimeString(String hour_minutes, uint8_t &hour, uint8_t &minutes)
{
    if (hour_minutes.length() != 5)
    {
        ESP_LOGE(TAG, "Invalid time string: %s", hour_minutes.c_str());
        return false;
    }
    hour = hour_minutes.substring(0, 2).toInt();
    minutes = hour_minutes.substring(3, 5).toInt();
    if (hour > 23)
    {
        ESP_LOGE(TAG, "Hour out of range: %d", hour);
        return false;
    }
    if (minutes > 59)
    {
        ESP_LOGE(TAG, "Minutes out of range: %d", minutes);
        return false;
    }
    return true;
}
