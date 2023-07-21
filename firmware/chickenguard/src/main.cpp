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
#include "display.h"
#include "wifi_credentials.h"

static const char *TAG = "Main";

#ifdef ARDUINO_USB_MODE
#warning "USB mode enabled"
#endif

static void displayWifiCredentials();
static void webConfigDone();
static void updateTime(long utc, const String timezone);
static void setOpenDoorAlarm(NonVolatileStorage::DoorControl const doorControl);
static void setCloseDoorAlarm(NonVolatileStorage::DoorControl const doorControl);
static void handleButtonPress(ButtonReader::ButtonSelection buttonState);

static TimeControl timeControl(readBytes, writeBytes);
static NonVolatileStorage config;
static Webservice webserver(&config, updateTime, webConfigDone);
// Voltage divider scale = (R306+R309)/R309
static powerControl power(powerControl::BatteryTech::Alkaline, 4, 4.03);
static MotorControl motor(MOTOR_IN1, MOTOR_IN2, MOTOR_CURRENT_SENSE);
static AsyncDelay rtcPollingDelay;
static ButtonReader button(SNS_BUTTON);
static Display display;
static bool motorRunning = false;
static AsyncDelay batteryStatusDelay;

void setup()
{
    /**
     * Only needed for reading from serial port (either UART0 or USB CDC)
     * Reading : In ARDUINO_USB_MODE, the USB-CDC is used, otherwise UART0 is used.
     * For log writing :  data is always sent to both UART0 and USB CDC.
     */
    Serial.begin(115200);
    while (!Serial)
        ;
    ESP_LOGD(TAG, "\r\nBuild %s, utc: %lu\r\n", COMMIT_HASH, CURRENT_TIME);

    power.init();
    motor.init(power.getVoltage_mV());
    config.restoreAll();
    assert(i2c_hal_init(I2C_SDA, I2C_SCL));
    display.init(delayMicroseconds);

    assert(timeControl.init(config.getTimeZone()));
    if (!timeControl.hasValidTime())
    {
        ESP_LOGE(TAG, "Time is not valid");
        // User will have to set the time using the webserver
        displayWifiCredentials();
        webserver.setup();
    }

    // Check if woken up by button press
    while (!button.isButtonStateStable())
    {
        delay(10);
    }
    ButtonReader::ButtonSelection buttonState = button.getButton();
    if (buttonState != ButtonReader::ButtonSelection::None)
    {
        handleButtonPress(buttonState);
    }
    ESP_LOGD(TAG, "Ready to rumble");
    batteryStatusDelay.start(1000, AsyncDelay::MILLIS);
}

void loop()
{
    webserver.loop();
    if(batteryStatusDelay.isExpired())
    {
        batteryStatusDelay.repeat();
        //ESP_LOGI(TAG, "Battery voltage: %d mV", power.getVoltage_mV());
        webserver.notifyClients("battery", String(power.getVoltage_percent()) + String("%"));
    }
    power.run();

    // Handle alarms
    if (rtcPollingDelay.isExpired() && timeControl.hasValidTime())
    {
        rtcPollingDelay.start(1000, AsyncDelay::MILLIS);

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
    if (button.update())
    {
        // Button state has changed
        handleButtonPress(button.getButton());
    }
    bool currentMotorRunning = motor.run();
    if (motorRunning && !currentMotorRunning)
    {
        // Motor has stopped
        ESP_LOGI(TAG, "Motor has stopped");
        power.powerOff();
    }
    motorRunning = currentMotorRunning;
}

void webConfigDone()
{
    ESP_LOGI(TAG, "Web config done");
    // Save all parameters
    config.saveAll();

    // Set the alarms
    setOpenDoorAlarm(config.getDoorControl());
    setCloseDoorAlarm(config.getDoorControl());

    // Power off
    power.powerOff();
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

void handleButtonPress(ButtonReader::ButtonSelection buttonState)
{
    switch (buttonState)
    {
    case ButtonReader::ButtonSelection::Down:
        ESP_LOGI(TAG, "Button pressed: Down");
        motor.closeDoor();
        break;
    case ButtonReader::ButtonSelection::Up:
        ESP_LOGI(TAG, "Button pressed: Up");
        motor.openDoor();
        break;
    case ButtonReader::ButtonSelection::Standby:
        ESP_LOGI(TAG, "Button pressed: Start webserver");
        displayWifiCredentials();
        webserver.setup();
        break;
    case ButtonReader::ButtonSelection::None:
    default:
        // Button released
        break;
    }
}

void displayWifiCredentials()
{
    char ssid[16];
    sprintf(ssid, "SSID: %s", WIFI_SSID);
    char password[16];
    sprintf(password, "Pass: %s", WIFI_PASS);
    display.show(ssid, password);
}