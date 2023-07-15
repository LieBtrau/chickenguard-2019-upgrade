#pragma once
#include <stdint.h>
#include <AsyncDelay.h>

class powerControl
{
public:
    enum class BatteryTech
    {
        Alkaline,
        NiMH
    };
    powerControl(const BatteryTech batteryTech, const uint32_t cellCount, const float voltageDividerScale);
    bool init();
    void run();
    uint32_t getVoltage_mV();
    uint32_t getVoltage_percent();
    bool isBatteryLow() const;
    void powerOff();
private:
    const BatteryTech _batteryTech;
    const uint32_t _cellCount;
    const float _voltageDividerScale;
    const unsigned long POWERED_ON_PERIOD = 180000;  //!< Device will power off after this amount of milliseconds.
    const unsigned long LED_BLINK_PERIOD = 200;     //!< LED will blink at this period.
    const uint32_t LOW_BATTERY_PERCENT = 20;        //!< Battery is considered low when it reaches this percentage.
    AsyncDelay _powerOnPeriod;
    AsyncDelay _ledBlinkDelay;
    bool _batteryLow = false;
};

