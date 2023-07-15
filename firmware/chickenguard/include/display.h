#pragma once

#include "LiquidCrystal_I2C.h"
#include "TCA9534.h"
#include "MCP40D18.h"

class Display
{
public:
    Display();
    ~Display();
    bool init(void (*delayFunc)(uint32_t));
    void show(const char *line1, const char *line2 = nullptr);

private:
    const int LCD_COLUMS = 16;
    const int LCD_ROWS = 2;
    TCA9534 _ioExpander;
    LiquidCrystal_I2C _lcd;
    MCP40D18 _potentiometer;
};