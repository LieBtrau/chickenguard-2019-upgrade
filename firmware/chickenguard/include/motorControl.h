#pragma once

#include <Arduino.h>
#include "AsyncDelay.h"
#include "RunningAverage.h"

class MotorControl {
    public:
        MotorControl(uint8_t pinIn1, uint8_t pinIn2, uint8_t pinCurrentSense);
        ~MotorControl();

        void init();
        bool run();
        void off();
        void demo();
        void raiseDoor(uint16_t duration_ms);
        void lowerDoor(uint16_t duration_ms);
    private:
        uint8_t _pinIn1;
        uint8_t _pinIn2;
        uint8_t _pinCurrentSense;
        AsyncDelay _motorOnTime;
        AsyncDelay _AdcSamplingPeriod;
        RunningAverage _currentSense;
};