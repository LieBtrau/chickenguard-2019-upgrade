#include "motorControl.h"

static const char *TAG = "MotorControl";

MotorControl::MotorControl(uint8_t pinIn1, uint8_t pinIn2, uint8_t pinCurrentSense) : 
_pinIn1(pinIn1), 
_pinIn2(pinIn2), 
_pinCurrentSense(pinCurrentSense),
_currentSense(20)
{
}

MotorControl::~MotorControl()
{
}

void MotorControl::init()
{
    pinMode(_pinIn1, OUTPUT);
    pinMode(_pinIn2, OUTPUT);
    off();
    _AdcSamplingPeriod.start(50, AsyncDelay::MILLIS);
}

/**
 * @brief Run the motor
 *
 * @return true when motor is running, else false
 */
bool MotorControl::run()
{
    if (_motorOnTime.isExpired())
    {
        off();
        return false;
    }
    // Motor is running
    if (_AdcSamplingPeriod.isExpired())
    {
        //Time to sample the current sense
        _AdcSamplingPeriod.repeat();
        _currentSense.addValue(analogReadMilliVolts(_pinCurrentSense));
        if(_currentSense.bufferIsFull())
        {
            ESP_LOGD(TAG, "Current sense: %2f", _currentSense.getAverage());
        }
        return true;
    }
    return true;
}

void MotorControl::raiseDoor(uint16_t duration_ms)
{
    digitalWrite(_pinIn1, LOW);
    digitalWrite(_pinIn2, HIGH);
    _motorOnTime.start(duration_ms, AsyncDelay::MILLIS);
    _currentSense.clear();
}

void MotorControl::lowerDoor(uint16_t duration_ms)
{
    digitalWrite(_pinIn1, HIGH);
    digitalWrite(_pinIn2, LOW);
    _motorOnTime.start(duration_ms, AsyncDelay::MILLIS);
    _currentSense.clear();
}

void MotorControl::off()
{
    digitalWrite(_pinIn1, LOW);
    digitalWrite(_pinIn2, LOW);
    _motorOnTime.expire();
}

void MotorControl::demo()
{
    ESP_LOGI(TAG, "Raise door");
    raiseDoor(10000);
    while (run())
        ;
    ESP_LOGI(TAG, "Lower door");
    lowerDoor(10000);
    while (run())
        ;
    off();
}