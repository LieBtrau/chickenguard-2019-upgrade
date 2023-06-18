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

static const char *TAG = "Main";

void setup()
{
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_DEBUG);
  ESP_LOGD(TAG, "\r\nBuild %s\r\n", __TIMESTAMP__);

  // Set timezone to Brussels time (source : last line of /usr/share/zoneinfo/Europe/Brussels)
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();

  int32_t unixtime = 1682781365; // result of linux cli : date '+%s'
  timeval epoch = {unixtime, 0};
  settimeofday((const timeval *)&epoch, 0);
}

void loop()
{
  time_t now;
  char strftime_buf[64];
  struct tm timeinfo;
  struct tm *ptm;

  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The current date/time in Brussels is: %s", strftime_buf);


  // Get hours and minutes from the current time as GMT-time
  ptm = gmtime(&now);
  ESP_LOGI(TAG, "GMT-time: %02d:%02d", ptm->tm_hour, ptm->tm_min);

  delay(5000);
}

void rtcUpdatesMcuTime()
{
  time_t now;
  time(&now);              // As MCU has no RTC, reading time will return 0.
  setenv("TZ", "UTC0", 1); // Set timezone to UTC (which is GMT)
  tzset();
  struct tm *timeinfo = localtime(&now);

  // Read time from RTC.  RTC operates in GMT time.

  // Update timeinfo with the time values that have been read from the RTC
  timeinfo->tm_year = 2021;
  timeinfo->tm_mon = 3;
  timeinfo->tm_mday = 29;
  timeinfo->tm_hour = 12;
  timeinfo->tm_min = 30;
  timeinfo->tm_sec = 0;
  time_t newtime = mktime(timeinfo);          // Convert to time_t using local timezone (that's why we set TZ to UTC0)
  timeval newepoch = {newtime, 0};
  settimeofday((const timeval *)&newepoch, 0); // Set the MCU time correctly

  // Correct the time zone
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();
}


/**
 * @brief Set the Alarm object
 * We have the hour and minutes in local time, but the RTC works in GMT time.
 * So we need to convert the local time to GMT time.
 */
void rtcSetAlarm()
{
  time_t now;
  time(&now);
  struct tm *timeinfo = localtime(&now);

  // Replace the hour and minutes with the alarm time
  timeinfo->tm_hour = 12;
  timeinfo->tm_min = 30;

  // Convert to GMT time
  time_t alarmtime = mktime(timeinfo);
  timeinfo = gmtime(&alarmtime);

  // Set the alarm
  uint8_t alarmHour = timeinfo->tm_hour;
  uint8_t alarmMinute = timeinfo->tm_min;
  uint8_t alarmSecond = 0;
}