#include "motorControl.h"

static const char *TAG = "MotorControl";

MotorControl::MotorControl(uint8_t pinIn1, uint8_t pinIn2, uint8_t pinCurrentSense) : _pinIn1(pinIn1),
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
    const unsigned long DEAD_TIME = 2000;
    const unsigned long RAISE_DOOR_TIME = 25000;
    const unsigned long LOWER_DOOR_TIME = 25000;
    const unsigned long LOOSE_ROPE_TIME = 5000;

    const float RAISING_UNDERLOAD_CURRENT = 1000;
    const float RAISING_OVERLOAD_CURRENT = 2000;
    const float LOWERING_OVERLOAD_CURRENT = 700;
    float current = 0;

    switch (_state)
    {
    case MotorState::Off:
        digitalWrite(_pinIn1, LOW);
        digitalWrite(_pinIn2, LOW);
        _motorOnTime.expire();
        return false;
    case MotorState::start_raise:
        digitalWrite(_pinIn1, LOW);
        digitalWrite(_pinIn2, HIGH);
        _motorOnTime.start(DEAD_TIME, AsyncDelay::MILLIS);
        _currentSense.clear();
        _direction = MotorDirection::Raise;
        _state = MotorState::dead_time;
        return true;
    case MotorState::start_lower:
        digitalWrite(_pinIn1, HIGH);
        digitalWrite(_pinIn2, LOW);
        _motorOnTime.start(DEAD_TIME, AsyncDelay::MILLIS);
        _currentSense.clear();
        _direction = MotorDirection::Lower;
        _state = MotorState::dead_time;
        return true;
    case MotorState::dead_time:
        // Wait for the motor to start and for the current to stabilize
        if (_motorOnTime.isExpired())
        {
            _motorOnTime.start((_direction == MotorDirection::Raise) ? RAISE_DOOR_TIME : LOWER_DOOR_TIME, AsyncDelay::MILLIS);
            _state = MotorState::running;
        }
        return true;
    case MotorState::raising_under_load:
        //Pull up loose rope until timeout or until the load is detected.
        if (_motorOnTime.isExpired())
        {
            //We won't be pulling up loose rope forever.
            _state = MotorState::Off;
        }
        if (readAdc(current))
        {
            ESP_LOGI(TAG, "Underload current: %f", current);
        }
        if(current > RAISING_UNDERLOAD_CURRENT)
        {
            ESP_LOGI(TAG, "Underload condition ended");
            _motorOnTime.start(RAISE_DOOR_TIME, AsyncDelay::MILLIS);
            _state = MotorState::running;
        }
        return true;
    case MotorState::running:
        if (_motorOnTime.isExpired())
        {
            _state = MotorState::Off;
        }
        if (readAdc(current))
        {
            ESP_LOGI(TAG, "Current: %f", current);
        }
        if(current > RAISING_OVERLOAD_CURRENT) 
        {
            ESP_LOGI(TAG, "Overload current detected");
            _state = MotorState::Off;
        }
        if(_direction == MotorDirection::Raise && current < RAISING_UNDERLOAD_CURRENT)
        {
            // The motor is pulling up loose rope.
            ESP_LOGI(TAG, "Raising underload current detected");
            _motorOnTime.start(LOOSE_ROPE_TIME, AsyncDelay::MILLIS);
            _state = MotorState::raising_under_load;
        }
        if(_direction == MotorDirection::Lower && current > LOWERING_OVERLOAD_CURRENT)
        {
            ESP_LOGI(TAG, "Lowering overload current detected");
            _state = MotorState::Off;
        }
        return true;
    default:
        _state = MotorState::Off;
        return false;
    }
}

void MotorControl::openDoor()
{
    _state = MotorState::start_raise;
}

void MotorControl::closeDoor()
{
    _state = MotorState::start_lower;
}

void MotorControl::off()
{
    _state = MotorState::Off;
}

bool MotorControl::readAdc(float &current)
{
    if (_AdcSamplingPeriod.isExpired())
    {
        _AdcSamplingPeriod.start(50, AsyncDelay::MILLIS);
        _currentSense.addValue(analogRead(_pinCurrentSense));
        current = _currentSense.getAverage();
        return true;
    }
    return false;
}

void MotorControl::demo()
{
    ESP_LOGI(TAG, "Raise door");
    openDoor();
    while (run())
        ;
    ESP_LOGI(TAG, "Lower door");
    closeDoor();
    while (run())
        ;
    off();
}