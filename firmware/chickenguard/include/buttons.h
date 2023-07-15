#pragma once

#include "AsyncDelay.h"

class ButtonReader
{
public:
    enum class ButtonSelection
    {
        Down,
        Standby,
        Up,
        None
    };
    ButtonReader(const int adcPin);
    ~ButtonReader();
    bool update();
    bool isButtonStateStable() { return _debounceDelay.isExpired(); }
    ButtonSelection getButton() { return _lastButtonState; }
private:
    ButtonSelection getPushedButton();
    const int _adcPin;
    ButtonSelection _lastButtonState;
    ButtonSelection _bouncingButtonState;
    AsyncDelay _debounceDelay;
};