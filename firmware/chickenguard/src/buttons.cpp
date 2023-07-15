#include "buttons.h"
#include <Arduino.h>

static const char *TAG = "Buttons";

ButtonReader::ButtonReader(const int adcPin) : _adcPin(adcPin),
                                               _lastButtonState(ButtonSelection::None),
                                               _bouncingButtonState(ButtonSelection::None)
{
}

ButtonReader::~ButtonReader()
{
}

/**
 * @brief Update the button state.
 * 
 * @return true if the button state has changed.
 */
bool ButtonReader::update()
{
    ButtonSelection buttonState = getPushedButton();
    if (buttonState != _bouncingButtonState)
    {
        //State is not stable
       _debounceDelay.start(50, AsyncDelay::MILLIS);
    }
    _bouncingButtonState = buttonState;
    if (_debounceDelay.isExpired())
    {
        //State is stable
        if(_bouncingButtonState != _lastButtonState)
        {
            _lastButtonState = _bouncingButtonState;
            return true;
        }
        //State is stable and has not changed
    }
    return false;
}

ButtonReader::ButtonSelection ButtonReader::getPushedButton()
{
    uint32_t adcValue = analogReadMilliVolts(_adcPin);
    const uint32_t MAX_BUTTON_DOWN_ADC_VALUE = 1100;
    const uint32_t MAX_BUTTON_STANDBY_ADC_VALUE = 2000;
    const uint32_t MAX_BUTTON_UP_ADC_VALUE = 2600;
    //ESP_LOGD(TAG, "ADC value: %d", adcValue);
    delay(100);
    if (adcValue < MAX_BUTTON_DOWN_ADC_VALUE)
    {
        // ADC value is 604mV when button is pressed
        return ButtonSelection::Down;
    }
    else if (adcValue < MAX_BUTTON_STANDBY_ADC_VALUE)
    {
        // ADC value is 1517 when button is pressed
        return ButtonSelection::Standby;
    }
    else if (adcValue < MAX_BUTTON_UP_ADC_VALUE)
    {
        // ADC value is 2400 when button is pressed
        return ButtonSelection::Up;
    }
    else
    {
        // ADC value is 2800mV when no button is pressed
        return ButtonSelection::None;
    }
}