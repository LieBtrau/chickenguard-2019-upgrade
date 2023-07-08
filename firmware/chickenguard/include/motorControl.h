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
        void openDoor();
        void closeDoor();
    private:
        enum class MotorState {
            Off,
            start_raise,
            start_lower,
            dead_time,
            raising_under_load,
            running
        };
        enum class MotorDirection {
            Raise,
            Lower,
            None
        };
        bool readAdc(float& current);
        uint8_t _pinIn1;
        uint8_t _pinIn2;
        uint8_t _pinCurrentSense;
        AsyncDelay _motorOnTime;
        AsyncDelay _AdcSamplingPeriod;
        RunningAverage _currentSense;
        MotorState _state  = MotorState::Off;
        MotorDirection _direction = MotorDirection::None;
};