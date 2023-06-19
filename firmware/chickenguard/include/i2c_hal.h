#pragma once

#include "stdint.h"
#include "stdbool.h"

bool i2c_hal_init(int sda, int scl);
bool detectI2cDevice(uint8_t i2c_address);
int8_t readBytes(uint8_t i2c_address, uint8_t reg, uint8_t size, uint8_t *data);
bool writeBytes(uint8_t i2c_address, uint8_t reg, uint8_t size, const uint8_t *data);
