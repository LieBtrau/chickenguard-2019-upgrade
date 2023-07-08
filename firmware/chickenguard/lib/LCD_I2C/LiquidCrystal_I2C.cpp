/**
 * @file LiquidCrystal_I2C.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-07-08
 *
 * @copyright Copyright (c) 2023
 *
 * [Datasheet of the LCD1602 module](https://www.vishay.com/docs/37484/lcd016n002bcfhet.pdf)
 *
 * Controllers : HD44780, ST7066U
 */

#include "LiquidCrystal_I2C.h"

#include "../../include/bit_manipulation.h"

LiquidCrystal_I2C::LiquidCrystal_I2C(IOexpander *io, uint8_t rs, uint8_t rw, uint8_t enable,
                                     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t backlight) : _io(io),
                                                                                                          _RS_bit(rs),
                                                                                                          _RW_bit(rw),
                                                                                                          _EN_bit(enable),
                                                                                                          _Backlight_bit(backlight)
{
  _data_pins[0] = d4;
  _data_pins[1] = d5;
  _data_pins[2] = d6;
  _data_pins[3] = d7;
}

void LiquidCrystal_I2C::config(uint8_t cols, uint8_t rows)
{
  _cols = cols < 80 ? cols : 80;
  _lines = rows < 4 ? rows : 4;

  uint8_t functionFlags = 0;

  _row_offsets[0] = 0x00;
  _row_offsets[1] = 0x40;
  _row_offsets[2] = 0x00 + cols;
  _row_offsets[3] = 0x40 + cols;

  if (rows > 1)
  {
    functionFlags |= 0x08;
  }

  _backlight = 0;

  // initializing the display
  _io->writePort(0x00);
  delayMicroseconds(40000);

  // sequence to reset. see "Initializing by Instruction" in datasheet
  // see https://github.com/arduino-libraries/LiquidCrystal/blob/master/src/LiquidCrystal.cpp#L121
  writeNibble(0x03, false);
  delayMicroseconds(4500);
  writeNibble(0x03, false);
  delayMicroseconds(4500);
  writeNibble(0x03, false);
  delayMicroseconds(150);
  writeNibble(0x02, false);
  delayMicroseconds(150);

  writeByte(LCD_FUNCTIONSET | LCD_4BITMODE | (rows > 1 ? LCD_2LINE : 0) | LCD_5x8DOTS, false);
  delayMicroseconds(150);
  displayOn(true);
  clear();
  textDirectionRightToLeft(true);
  autoScroll(false);
}

void LiquidCrystal_I2C::init(void (*delayFunc)(uint32_t))
{
  _delay_us = delayFunc;
}

void LiquidCrystal_I2C::delayMicroseconds(uint32_t us)
{
  assert(_delay_us != nullptr);
  _delay_us(us);
}

void LiquidCrystal_I2C::clear()
{
  writeByte(LCD_CLEARDISPLAY);
  delayMicroseconds(1600); // this command takes 1.5ms!
} 

void LiquidCrystal_I2C::home()
{
  writeByte(LCD_CURSORHOME);
  delayMicroseconds(1600); // this command takes 1.5ms!
} 


void LiquidCrystal_I2C::setCursor(uint8_t col, uint8_t row)
{
  // check boundaries
  assert((col < _cols) && (row < _lines));
  if ((col < _cols) && (row < _lines))
  {
    writeByte(LCD_WRITERAM | (_row_offsets[row] + col));
  }
}

void LiquidCrystal_I2C::displayOn(bool on)
{
  writeByte(LCD_DISPLAYCONTROL | (on ? LCD_DISPLAYON : 0));
}


void LiquidCrystal_I2C::cursorOn(bool on)
{
  writeByte(LCD_DISPLAYCONTROL | (on ? LCD_CURSORON : 0));
} 

void LiquidCrystal_I2C::blinkOn(bool on)
{
  writeByte(LCD_DISPLAYCONTROL | (on ? LCD_BLINKON : 0));
} 

void LiquidCrystal_I2C::scrollDisplayRight(bool isRight)
{
  writeByte(LCD_SHIFT | LCD_DISPLAYMOVE | (isRight ? LCD_MOVERIGHT : LCD_MOVELEFT));
} 


void LiquidCrystal_I2C::textDirectionRightToLeft(bool leftToRight)
{
  writeByte(LCD_ENTRYMODESET | (leftToRight ? LCD_ENTRYLEFT : 0));
} 


void LiquidCrystal_I2C::autoScroll(bool on)
{
  writeByte(LCD_ENTRYMODESET | (on ? LCD_AUTOSCROLL : 0));
} 

void LiquidCrystal_I2C::setBacklight(uint8_t brightness)
{
  uint8_t data = 0;
  _backlight = brightness;
  // send no data but set the background-pin right;
  bitWrite(data, _Backlight_bit, _backlight > 0);
  _io->writePort(data);
} 

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal_I2C::createChar(uint8_t location, uint8_t charmap[])
{
  location &= 0x7; // we only have 8 locations 0-7
  // Set CGRAM address
  writeByte(0x40 | (location << 3));
  for (uint8_t i = 0; i < 8; i++)
  {
    write(charmap[i]);
  }
}

/**
 * @brief write a single character to the display
 * 
 */
inline size_t LiquidCrystal_I2C::write(uint8_t ch)
{
  writeByte(ch, true);
  return 1; // assume success
}

// write either command or data
void LiquidCrystal_I2C::writeByte(uint8_t value, bool isData)
{
  writeNibble((value >> 4 & 0x0F), isData);
  delayMicroseconds(40);
  writeNibble((value & 0x0F), isData);
  // All commands have a delay of at least 39us.
  delayMicroseconds(40);
} 

/**
 * @brief Write a nibble to the display. The nibble is written to the data pins
 * @details Writing takes two I2C transmissions. The first one sets the data pins with the enable pin high, the second one sets the enable pin low.
 *  Data is clocked on the falling edge of the enable pin.
 * @param halfByte
 * @param isData
 * @param data
 */
void LiquidCrystal_I2C::writeNibble(uint8_t halfByte, bool isData)
{
  uint8_t data[2] = {0, 0};

  // map the data to the given pin connections
  bitWrite(data[0], _RS_bit, isData);
  // _rw_mask is not used here.
  bitWrite(data[0], _Backlight_bit, _backlight > 0);

  // allow for arbitrary pin configuration
  bitWrite(data[0], _data_pins[0], bitRead(halfByte, 0));
  bitWrite(data[0], _data_pins[1], bitRead(halfByte, 1));
  bitWrite(data[0], _data_pins[2], bitRead(halfByte, 2));
  bitWrite(data[0], _data_pins[3], bitRead(halfByte, 3));

  data[1] = data[0];
  bitSet(data[0], _EN_bit);
  _io->writePort(data[0]);
  delayMicroseconds(1); // enable pulse must be >450ns
  _io->writePort(data[1]);
}
