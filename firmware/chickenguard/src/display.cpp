#include "display.h"
#include "i2c_hal.h"

Display::Display() : _ioExpander(true, true, true), _lcd(&_ioExpander), _potentiometer()
{
}

Display::~Display()
{
}

bool Display::init(void (*delayFunc)(uint32_t))
{
    assert(detectI2cDevice(_ioExpander.getI2cAddress()));
    _ioExpander.attach(readBytes, writeBytes);
    _ioExpander.setPortDirection(0x00); // 0x00 = All outputs (yes, the TCA9534 is inverted)

    assert(detectI2cDevice(_potentiometer.getI2cAddress()));
    _potentiometer.attach(readBytes, writeBytes);

    _lcd.init(delayFunc);
    _lcd.config(LCD_COLUMS, LCD_ROWS);
    _lcd.clear();
    _potentiometer.setWiper(16); // 16 gives best LCD contrast
    return true;
}

void Display::show(const char *line1, const char *line2)
{
    _lcd.clear();
    _lcd.home();
    _lcd.setBacklight(true);
    _lcd.print(line1);
    if (line2 == nullptr)
    {
        return;
    }
    _lcd.setCursor(0, 1);
    _lcd.print(line2);
}

void Display::off()
{
    _lcd.setBacklight(false);
    _lcd.clear();
    _lcd.home();
}