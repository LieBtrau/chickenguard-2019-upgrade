/**
 */

#include "pins.h"
#include "Webservice.h"
#include <AsyncDelay.h>
#include "timeControl.h"
#include "NonVolatileStorage.h"
#include "i2c_hal.h"
#include "DS1337.h"
#include "VoltageMonitor.h"

static const char *TAG = "Main";

AsyncDelay timeShowDelay;
AsyncDelay ledBlinkDelay;
TimeControl timeControl;
NonVolatileStorage config;
DS1337 rtc(readBytes, writeBytes);
//Voltage divider scale = (R306+R309)/R309
VoltageMonitor monitorVmotor(4.03, SNS_VMOTOR);

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
    // setupWebserver();
    // config.restoreAll();
    timeShowDelay.start(5000, AsyncDelay::MILLIS);
    ledBlinkDelay.start(1000, AsyncDelay::MILLIS);

    assert(i2c_hal_init(I2C_SDA, I2C_SCL));
    assert(detectI2cDevice(rtc.getI2cAddress()));

    // int32_t unixtime = 1687093084; // result of linux cli : date '+%s'
    // timeval epoch = {unixtime, 0};
    // settimeofday((const timeval *)&epoch, 0);
    // time_t now;
    // time(&now);
    // assert(rtc.setTime(now));
    // rtc.enableSquareWave(false); // disable square wave to save power
}


void loop()
{
    // loopWebserver();
    // if (timeShowDelay.isExpired())
    // {
    //     timeShowDelay.repeat();
    //     time_t now;
    //     char strftime_buf[64];
    //     struct tm timeinfo;

    //     time(&now);
    //     localtime_r(&now, &timeinfo);
    //     strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    //     ESP_LOGI(TAG, "The current date/time in Brussels is: %s", strftime_buf);
    // }
    if (ledBlinkDelay.isExpired())
    {
        ledBlinkDelay.repeat();
        //digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        ESP_LOGI(TAG, "Battery status: %d%%", monitorVmotor.getVoltage_percent(VoltageMonitor::BatteryTech::Alkaline, 4));
        //notifyClients("battery",String(i));
    }
}