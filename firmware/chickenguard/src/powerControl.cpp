#include "VoltageMonitor.h"
#include "pins.h"

VoltageMonitor::VoltageMonitor(const float voltageDividerScale, const uint8_t pin) : _voltageDividerScale(voltageDividerScale),
                                                                                     _pin(pin)
{
}

uint32_t VoltageMonitor::getVoltage_mV()
{
    const int SAMPLE_COUNT = 10;
    int adcValue = 0;
    const uint32_t MAX_mV_MEASUREMENT = 3500;

    int sampleCount=0;
    while(sampleCount < SAMPLE_COUNT)
    {
        uint32_t measurement = analogReadMilliVolts(_pin);
        if (measurement < MAX_mV_MEASUREMENT)
        {
            adcValue += measurement;
            sampleCount++;
        }
        delay(1);
    }
    return adcValue * _voltageDividerScale / SAMPLE_COUNT;
}

uint32_t VoltageMonitor::getVoltage_percent(BatteryTech batteryTech, uint32_t cellCount)
{
    uint32_t voltage = getVoltage_mV() / cellCount;
    uint32_t minVoltage = 0;
    uint32_t maxVoltage = 0;
    switch (batteryTech)
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