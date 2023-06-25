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

TimeControl timeControl(readBytes, writeBytes);
NonVolatileStorage config;

// Voltage divider scale = (R306+R309)/R309
powerControl power(powerControl::BatteryTech::Alkaline, 4, 4.03);

#ifdef ARDUINO_USB_MODE
#warning "USB mode enabled"
#endif

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

    if (!timeControl.hasValidTime())
    {
        ESP_LOGE(TAG, "Time is not valid");
        // User will have to set the time using the webserver
        setupWebserver();
    }
}

void loop()
{
    loopWebserver();
    power.run();
}