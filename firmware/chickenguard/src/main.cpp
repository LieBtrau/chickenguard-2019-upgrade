/**
 */

#include "pins.h"
#include "Webservice.h"
#include <AsyncDelay.h>
#include "timeControl.h"
#include "NonVolatileStorage.h"
#include "i2c_hal.h"
#include "powerControl.h"

static const char *TAG = "Main";

#ifdef ARDUINO_USB_MODE
#warning "USB mode enabled"
#endif

static void liftDoor();
static void lowerDoor();
static void webConfigDone();
static void updateTime(long utc, const String timezone);

static TimeControl timeControl(readBytes, writeBytes, liftDoor, lowerDoor);
static NonVolatileStorage config;
static Webservice webserver(&config, updateTime, webConfigDone);
// Voltage divider scale = (R306+R309)/R309
static powerControl power(powerControl::BatteryTech::Alkaline, 4, 4.03);

void setup()
{

    power.init();

    /**
     * Only needed for reading from serial port (either UART0 or USB CDC)
     * Reading : In ARDUINO_USB_MODE, the USB-CDC is used, otherwise UART0 is used.
     * For log writing :  data is always sent to both UART0 and USB CDC.
     */
    Serial.begin(115200);
    while (!Serial)
        ;

    ESP_LOGD(TAG, "\r\nBuild %s\r\n", __TIMESTAMP__);

    config.restoreAll();
    assert(i2c_hal_init(I2C_SDA, I2C_SCL));
    assert(timeControl.init(config.getTimeZone()));

    // if (!timeControl.hasValidTime())
    // {
    //     ESP_LOGE(TAG, "Time is not valid");
        // User will have to set the time using the webserver
        webserver.setup();
    // }
}

void loop()
{
    webserver.loop();
    power.run();
    timeControl.run();
}

void liftDoor()
{
    ESP_LOGI(TAG, "Lift door");
    switch (config.getDoorControl())
    {
    case NonVolatileStorage::DoorControl::SunriseSunset:
        float latitude, longitude;
        config.getGeoLocation(latitude, longitude);
        timeControl.setCloseAlarmSunset(latitude, longitude);
        break;
    case NonVolatileStorage::DoorControl::FixTime:
        //Update needed, because the daylightsaving time might have changed
        //We have to adapt to daylight saving time, so do the chickens.
        uint8_t hr, min;
        config.getFixClosingTime(hr, min);
        assert(timeControl.setCloseAlarmFixTime(hr, min));
        break;
    default:
        break;
    }
}

void lowerDoor()
{
    ESP_LOGI(TAG, "Lower door");
    switch(config.getDoorControl())
    {
        case NonVolatileStorage::DoorControl::SunriseSunset:
            float latitude, longitude;
            config.getGeoLocation(latitude, longitude);
            timeControl.setOpenAlarmSunrise(latitude, longitude);
            break;
        case NonVolatileStorage::DoorControl::FixTime:
            //Update needed, because the daylightsaving time might have changed
            //We have to adapt to daylight saving time, so do the chickens.
            uint8_t hr, min;
            config.getFixOpeningTime(hr, min);
            assert(timeControl.setOpenAlarmFixTime(hr, min));
            break;
        default:
            break;
    }
}

void webConfigDone()
{
    ESP_LOGI(TAG, "Web config done");
    // Save all parameters
    config.saveAll();

    switch (config.getDoorControl())
    {
    case NonVolatileStorage::DoorControl::SunriseSunset:
        float latitude, longitude;
        config.getGeoLocation(latitude, longitude);
        timeControl.setOpenAlarmSunrise(latitude, longitude);
        timeControl.setCloseAlarmSunset(latitude, longitude);
        break;
    case NonVolatileStorage::DoorControl::FixTime:
        uint8_t hr, min;
        config.getFixOpeningTime(hr, min);
        assert(timeControl.setOpenAlarmFixTime(hr, min));
        config.getFixClosingTime(hr, min);
        assert(timeControl.setCloseAlarmFixTime(hr, min));
        break;
    case NonVolatileStorage::DoorControl::Manual:
    default:
        timeControl.disableAlarms();
        break;
    }
}

void updateTime(long utc, const String timezone)
{
    ESP_LOGI(TAG, "Update time to UTC-seconds: %ld & timezone %s", utc, timezone.c_str());
    config.setTimeZone(timezone);
    timeControl.updateMcuTime(utc, timezone);
}