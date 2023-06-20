/**
 */

#include "pins.h"
#include "Webservice.h"
#include <AsyncDelay.h>
#include "timeControl.h"
#include "NonVolatileStorage.h"
#include "i2c_hal.h"
#include "DS1337.h"

static const char *TAG = "Main";

AsyncDelay timeShowDelay;
AsyncDelay ledBlinkDelay;
TimeControl timeControl;
NonVolatileStorage config;
DS1337 rtc(readBytes, writeBytes);

#ifdef ARDUINO_USB_MODE
#warning "USB mode enabled"
#endif

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // turn LED on, don't wait for timer to expire first.
    pinMode(EN_PWR, OUTPUT);
    digitalWrite(EN_PWR, HIGH); // take over power enable pin from momentary switch.turn LED on, don't wait for timer to expire first.

    /**
     * Only needed for reading from serial port (either UART0 or USB CDC)
     * In ARDUINO_USB_MODE, the USB CDC is used for reading, otherwise UART0 is used.  For writing, data is always sent to both
     * UART0 and USB CDC.
     */
    Serial.begin(115200);
    while (!Serial)
        ;

    ESP_LOGD(TAG, "\r\nBuild %s\r\n", __TIMESTAMP__);
    setupWebserver();
    config.restoreAll();
    timeShowDelay.start(5000, AsyncDelay::MILLIS);
    //ledBlinkDelay.start(500, AsyncDelay::MILLIS);

    if (!i2c_hal_init(9, 8))
    {
        ESP_LOGE(TAG, "I2C init failed");
        while (1)
            ;
    }

    if (!detectI2cDevice(rtc.getI2cAddress()))
    {
        ESP_LOGE(TAG, "DS1337 not found");
        while (1)
            ;
    }
    ESP_LOGI(TAG, "DS1337 found");

    int32_t unixtime = 1687093084; // result of linux cli : date '+%s'
    timeval epoch = {unixtime, 0};
    settimeofday((const timeval *)&epoch, 0);
    time_t now;
    time(&now);
    if (!rtc.setTime(now))
    {
        ESP_LOGE(TAG, "Failed to set time");
        while (1)
            ;
    }
    rtc.enableSquareWave(false); // disable square wave to save power
}

void loop()
{
    loopWebserver();
    if (timeShowDelay.isExpired())
    {
        timeShowDelay.repeat();
        time_t now;
        char strftime_buf[64];
        struct tm timeinfo;

        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in Brussels is: %s", strftime_buf);
    }
    // if (ledBlinkDelay.isExpired())
    // {
    //     ledBlinkDelay.repeat();
    //     digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    // }
    if(Serial.available())
    {
        char c = Serial.read();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}