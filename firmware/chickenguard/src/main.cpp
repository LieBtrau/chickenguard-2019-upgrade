/**
 */

#include "pins.h"
#include "Webservice.h"
#include <AsyncDelay.h>
#include "timeControl.h"
#include "NonVolatileStorage.h"
#include "i2c_hal.h"
#include "VoltageMonitor.h"

static const char *TAG = "Main";

AsyncDelay timeShowDelay;
AsyncDelay ledBlinkDelay;
TimeControl timeControl(readBytes, writeBytes);
NonVolatileStorage config;

// Voltage divider scale = (R306+R309)/R309
VoltageMonitor monitorVmotor(4.03, SNS_VMOTOR);

#ifdef ARDUINO_USB_MODE
#warning "USB mode enabled"
#endif

void setup()
{

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // turn LED on, don't wait for timer to expire first.
    pinMode(EN_PWR, OUTPUT);
    digitalWrite(EN_PWR, HIGH); // take over power enable pin from momentary switch to keep power on when user releases button.

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
        setupWebserver();
    }
    timeShowDelay.start(5000, AsyncDelay::MILLIS);
    ledBlinkDelay.start(1000, AsyncDelay::MILLIS);
}

void loop()
{
    loopWebserver();
    if (ledBlinkDelay.isExpired())
    {
        ledBlinkDelay.repeat();
        // digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        ESP_LOGI(TAG, "Battery status: %d%%", monitorVmotor.getVoltage_percent(VoltageMonitor::BatteryTech::Alkaline, 4));
        // notifyClients("battery",String(i));
    }
}