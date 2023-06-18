#include <Arduino.h>
#include <Wire.h>
#include "i2c_hal.h"
#include "DS1337.h"
#include "esp_log.h"
#include "AsyncDelay.h"

DS1337 rtc(readBytes, writeBytes);
static const char *TAG = "main";
AsyncDelay asyncDelay;

void setup()
{
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_DEBUG);
  ESP_LOGD(TAG, "\r\nBuild %s\r\n", __TIMESTAMP__);
  while (!Serial)
    ;
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
  asyncDelay.start(5000, AsyncDelay::MILLIS);
  rtc.enableSquareWave(false); // disable square wave to save power

  rtc.setDailyAlarm1(12,59);
  ESP_LOGI(TAG, "Done");
}

void loop()
{
  time_t now;
  if (asyncDelay.isExpired())
  {
    asyncDelay.repeat();
    if (!rtc.getTime(&now))
    {
      ESP_LOGE(TAG, "Failed to get time");
      while (1)
        ;
    }
    if(rtc.isAlarm1Triggered())
    {
      ESP_LOGI(TAG, "Alarm 1 triggered");
      rtc.acknowledgeAlarm1();
    }
    //tm *timeinfo = localtime(&now);
    // ESP_LOGI(TAG, "The current date/time is: %s", asctime(timeinfo));
  }
}
