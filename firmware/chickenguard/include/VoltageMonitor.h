#pragma once
#include <stdint.h>

class VoltageMonitor
{
public:
    enum class BatteryTech
    {
        Alkaline,
        NiMH
    };
    VoltageMonitor(const float voltageDividerScale, const uint8_t pin);
    uint32_t getVoltage_mV();
    uint32_t getVoltage_percent(BatteryTech batteryTech, uint32_t cellCount);
private:
    const float _voltageDividerScale;
    const uint8_t _pin;
};

