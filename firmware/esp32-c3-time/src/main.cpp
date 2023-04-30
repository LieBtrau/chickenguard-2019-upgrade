/**
 * @file main.cpp
 * @author your name (you@domain.com)
 * @brief Work with time and date
 * @version 0.1
 * @date 2023-04-29
 * 
 * @copyright Copyright (c) 2023
 * 
 * https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32c3/api-reference/system/system_time.html
 * https://github.com/lbernstone/ESP32_settimeofday/blob/master/settimeofday.ino
 */

#include <Arduino.h>

static const char* TAG = "Main";
//ESP-IDF has an option to set this, but we're using PlatformIO.
#define CONFIG_LOG_MAXIMUM_LEVEL ESP_LOG_DEBUG

void setup()
{
  esp_log_level_set("*", ESP_LOG_DEBUG);
  ESP_LOGD(TAG, "\r\nBuild %s\r\n", __TIMESTAMP__);

  // Set timezone to Brussels time (source : last line of /usr/share/zoneinfo/Europe/Brussels)
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();

  int32_t unixtime = 1682781365; // result of linux cli : date '+%s'
  timeval epoch = {unixtime, 0};
  settimeofday((const timeval*)&epoch, 0);
}

void loop()
{
  time_t now;
  char strftime_buf[64];
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The current date/time in Brussels is: %s", strftime_buf);
  delay(1000);
}