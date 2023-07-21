#include "powerControl.h"
#include "pins.h"

static const char *TAG = "powerControl";

powerControl::powerControl(const BatteryTech batteryTech, const uint32_t cellCount, const float voltageDividerScale) : _batteryTech(batteryTech),
                                                                                                                       _cellCount(cellCount),
                                                                                                                       _voltageDividerScale(voltageDividerScale)
{
}

bool powerControl::init()
{
    pinMode(EN_PWR, OUTPUT);
    digitalWrite(EN_PWR, HIGH); // take over power enable pin from momentary switch to keep power on when user releases button.
    _powerOnPeriod.start(POWERED_ON_PERIOD, AsyncDelay::MILLIS);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // turn LED on, don't wait for timer to expire first.
    _ledBlinkDelay.start(LED_BLINK_PERIOD, AsyncDelay::MILLIS);

    return true;
}

void powerControl::run()
{
    if (_powerOnPeriod.isExpired())
    {
        ESP_LOGD(TAG, "Time on period expired");
        powerOff();
    }
    if (_ledBlinkDelay.isExpired())
    {
        _ledBlinkDelay.repeat();
        if (getVoltage_percent() < LOW_BATTERY_PERCENT)
        {
            ESP_LOGE(TAG, "Battery voltage is too low");
            _batteryLow = true;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
        else
        {
            _batteryLow = false;
            digitalWrite(LED_PIN, LOW); // turn LED off
        }
    }
}

uint32_t powerControl::getVoltage_mV()
{
    const int SAMPLE_COUNT = 10;
    int adcValue = 0;
    const uint32_t MAX_mV_MEASUREMENT = 3500;

    int sampleCount = 0;
    while (sampleCount < SAMPLE_COUNT)
    {
        uint32_t measurement = analogReadMilliVolts(SNS_VMOTOR);
        if (measurement < MAX_mV_MEASUREMENT)
        {
            adcValue += measurement;
            sampleCount++;
        }
        delay(1);
    }
    return adcValue * _voltageDividerScale / SAMPLE_COUNT;
}

/**
 * @brief Get the Voltage percent object
 * @details Get the voltage as a percentage of the battery technology and cell count.  The implementation is a suggestion of Github Copilot.
 * @param batteryTech BatteryTech::Alkaline or BatteryTech::NiMH
 * @param cellCount Number of battery cells in series.
 * @return the voltage as a percentage [0..100] of the battery technology and cell count.
 */
uint32_t powerControl::getVoltage_percent()
{
    uint32_t voltage = getVoltage_mV() / _cellCount;
    uint32_t minVoltage = 0;
    uint32_t maxVoltage = 0;
    switch (_batteryTech)
    {
    case BatteryTech::Alkaline:
        minVoltage = 900;
        maxVoltage = 1500;
        break;
    case BatteryTech::NiMH:
        minVoltage = 1000;
        maxVoltage = 1200;
        break;
    }
    uint32_t range = maxVoltage - minVoltage;
    uint32_t voltageInRange = voltage - minVoltage;
    uint32_t percent = voltageInRange * 100 / range;
    return percent;
}

bool powerControl::isBatteryLow() const
{
    return _batteryLow;
}

void powerControl::powerOff()
{
    ESP_LOGI(TAG, "Powering off");
    delay(100); // wait for serial output to finish
    /**
     * @brief Power off the device when powered from the battery.
     */
    digitalWrite(EN_PWR, LOW);
    delay(100); // wait for unit to power off

    // In case USB or debug-port is connected, the device will stay powered and we will arrive here.
    // Put the device in deep sleep to save power.
    ESP_LOGD(TAG, "Entering deep sleep");
    esp_deep_sleep_start();

    // Zzz....zzz....
}