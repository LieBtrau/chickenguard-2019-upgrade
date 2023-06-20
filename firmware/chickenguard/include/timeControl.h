#pragma once

#include <Arduino.h>

class TimeControl
{
public:
    TimeControl();
    ~TimeControl();

    bool updateMcuTime(long utc, const String timeZone);

private:
    bool _timeIsSet=false;
    int _timeZoneIndex=0;
};