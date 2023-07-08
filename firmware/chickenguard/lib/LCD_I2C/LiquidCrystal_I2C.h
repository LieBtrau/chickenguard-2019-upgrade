#pragma once

#include "Print.h"
#include <stddef.h>
#include <stdint.h>

class IOexpander
{
public:
  virtual bool writePort(uint8_t value) const = 0;
};

class LiquidCrystal_I2C : public Print
{
public:
  LiquidCrystal_I2C(IOexpander *io, uint8_t rs=0, uint8_t rw=1, uint8_t enable=2,
                        uint8_t d4=4, uint8_t d5=5, uint8_t d6=6, uint8_t d7=7, uint8_t backlight = 3);
  void config(uint8_t cols=16, uint8_t rows=2);
  void init(void (*delayFunc)(uint32_t));
  void clear();
  void home();
  void setCursor(uint8_t col, uint8_t row);
  void cursorOn(bool on);
  void blinkOn(bool on);
  void displayOn(bool on);
  void scrollDisplayRight(bool isRight);
  void autoScroll(bool on);
  void textDirectionRightToLeft(bool leftToRight);
  void createChar(uint8_t location, uint8_t charmap[]);

  // plus functions from LCDAPI:
  void setBacklight(uint8_t brightness);
  inline void command(uint8_t value) { writeByte(value); }

  // support of Print class
  size_t write(uint8_t ch) override;

private:
  // Function set commands
  const uint8_t LCD_FUNCTIONSET = 0x20;
  const uint8_t LCD_4BITMODE = 0x00;
  const uint8_t LCD_2LINE = 0x08;
  const uint8_t LCD_5x8DOTS = 0x00;

  // Shift control commands
  const uint8_t LCD_SHIFT = 0x10;
  const uint8_t LCD_CURSORMOVE = 0x00;
  const uint8_t LCD_DISPLAYMOVE = 0x08;
  const uint8_t LCD_MOVERIGHT = 0x04;
  const uint8_t LCD_MOVELEFT = 0x00;

  // Display control commands
  const uint8_t LCD_DISPLAYCONTROL = 0x08;
  const uint8_t LCD_DISPLAYON = 0x04;
  const uint8_t LCD_CURSORON = 0x02;
  const uint8_t LCD_BLINKON = 0x01;

  // Clear display command
  const uint8_t LCD_CLEARDISPLAY = 0x01;

  // Cursor control commands
  const uint8_t LCD_CURSORHOME = 0x02;

  // Entry mode set commands
  const uint8_t LCD_ENTRYMODESET = 0x04;
  const uint8_t LCD_ENTRYLEFT = 0x02;
  const uint8_t LCD_AUTOSCROLL = 0x01;

  // Memory address set commands
  const uint8_t LCD_WRITERAM = 0x80;

  // instance variables
  IOexpander *_io;
  uint8_t _backlight;      ///< the backlight intensity
  uint8_t _cols;           ///< number of cols of the display
  uint8_t _lines;          ///< number of lines of the display
  uint8_t _entrymode;      ///< flags from entrymode
  uint8_t _displaycontrol; ///< flags from displaycontrol
  uint8_t _row_offsets[4];

  // variables describing how the PCF8574 is connected to the LCD
  uint8_t _RS_bit; // LOW: command.  HIGH: character.
  uint8_t _RW_bit; // LOW: write to LCD.  HIGH: read from LCD. <= to check
  uint8_t _EN_bit;  
  uint8_t _data_pins[4];// these are used for 4-bit data to the display.
  uint8_t _Backlight_bit;
  

  // low level function
  void (*_delay_us)(uint32_t us) = nullptr;
  void delayMicroseconds(uint32_t us);
  void writeByte(uint8_t value, bool isData = false);
  void _sendNibble(uint8_t halfByte, bool isData = false);
  void writeNibble(uint8_t halfByte, bool isData);
};
