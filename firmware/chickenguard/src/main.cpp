/**
 */

#include "pins.h"
#include "Webservice.h"
#include <AsyncDelay.h>
#include "timeControl.h"
#include "NonVolatileStorage.h"
#include "i2c_hal.h"
#include "powerControl.h"
#include "motorControl.h"
#include "buttons.h"

static const char *TAG = "Main";

#ifdef ARDUINO_USB_MODE
#warning "USB mode enabled"
#endif

static void openDoor();
static void closeDoor();
static void webConfigDone();
static void updateTime(long utc, const String timezone);
static void setOpenDoorAlarm(NonVolatileStorage::DoorControl const doorControl);
static void setCloseDoorAlarm(NonVolatileStorage::DoorControl const doorControl);

static TimeControl timeControl(readBytes, writeBytes);
static NonVolatileStorage config;
static Webservice webserver(&config, updateTime, webConfigDone);
// Voltage divider scale = (R306+R309)/R309
static powerControl power(powerControl::BatteryTech::Alkaline, 4, 4.03);
static MotorControl motor(MOTOR_IN1, MOTOR_IN2, MOTOR_CURRENT_SENSE);
static AsyncDelay rtcUpdateDelay;
static ButtonReader button(SNS_BUTTON);

void setup()
{
    motor.init();
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

    if (!timeControl.hasValidTime())
    {
        ESP_LOGE(TAG, "Time is not valid");
        // User will have to set the time using the webserver
        webserver.setup();
    }
}

void loop()
{
    webserver.loop();
    power.run();

    // Handle alarms
    if (rtcUpdateDelay.isExpired() && timeControl.hasValidTime())
    {
        rtcUpdateDelay.start(1000, AsyncDelay::MILLIS);

        if (timeControl.openDoorAlarmTriggered())
        {
            setCloseDoorAlarm(config.getDoorControl());
            motor.openDoor();
        }
        else if (timeControl.closeDoorAlarmTriggered())
        {
            // Update the sunrise alarm
            setOpenDoorAlarm(config.getDoorControl());
            motor.closeDoor();
        }
    }

    motor.run();
    if(button.update())
    {
        // Button state has changed (key press, key release)
        switch(button.getButton())
        {
            case ButtonReader::ButtonSelection::Down:
                motor.closeDoor();
                break;
            case ButtonReader::ButtonSelection::Up:
                motor.openDoor();
                break;
            case ButtonReader::ButtonSelection::Standby:
                webserver.setup();
                break;
            case ButtonReader::ButtonSelection::None:
            default:
                // Button released
                break;
        }

    }
}

void webConfigDone()
{
    ESP_LOGI(TAG, "Web config done");
    // Save all parameters
    config.saveAll();

    // Set the alarms
    setOpenDoorAlarm(config.getDoorControl());
    setCloseDoorAlarm(config.getDoorControl());
}

void updateTime(long utc, const String timezone)
{
    ESP_LOGI(TAG, "Update time to UTC-seconds: %ld & timezone %s", utc, timezone.c_str());
    config.setTimeZone(timezone);
    timeControl.updateMcuTime(utc, timezone);
}

/**
 * @brief Program the alarm to open the door
 * 
 * @param doorControl : controlled by sunrise/sunset or fix time
 */
void setOpenDoorAlarm(NonVolatileStorage::DoorControl const doorControl)
{
    switch (doorControl)
    {
    case NonVolatileStorage::DoorControl::SunriseSunset:
        float latitude, longitude;
        config.getGeoLocation(latitude, longitude);
        timeControl.setOpenAlarmSunrise(latitude, longitude);
        break;
    case NonVolatileStorage::DoorControl::FixTime:
        // Update needed, because the daylightsaving time might have changed
        // We have to adapt to daylight saving time, so do the chickens.
        uint8_t hr, min;
        config.getFixOpeningTime(hr, min);
        assert(timeControl.setOpenAlarmFixTime(hr, min));
        break;
    default:
        timeControl.disableAlarms();
        break;
    }
}

/**
 * @brief Program the alarm to close the door
 * 
 * @param doorControl : controlled by sunrise/sunset or fix time
 */
void setCloseDoorAlarm(NonVolatileStorage::DoorControl const doorControl)
{
    switch (doorControl)
    {
    case NonVolatileStorage::DoorControl::SunriseSunset:
        float latitude, longitude;
        config.getGeoLocation(latitude, longitude);
        timeControl.setCloseAlarmSunset(latitude, longitude);
        break;
    case NonVolatileStorage::DoorControl::FixTime:
        // Update needed, because the daylightsaving time might have changed
        // We have to adapt to daylight saving time, so do the chickens.
        uint8_t hr, min;
        config.getFixClosingTime(hr, min);
        assert(timeControl.setCloseAlarmFixTime(hr, min));
        break;
    default:
        timeControl.disableAlarms();
        break;
    }
}