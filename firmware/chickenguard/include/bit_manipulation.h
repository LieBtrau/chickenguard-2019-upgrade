#define lowByte(w) ((uint8_t)((w)&0xff))
#define highByte(w) ((uint8_t)((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

/**
 * @brief Macro to define a bit field in a register.
 * @param msb Most significant bit position
 * @param lsb Least significant bit position
 * @details  The problem with bitfields in C is that the order of the bits is not defined.
 * This macro defines a bit field with fixed order and position.
 * high nibble = width of bit field
 * low nibble = lowest bit position
 * 
 * In your implementation, you can use it like this:
 * enum UART0_CR_bits
 * {
 *   interrupt_enable  = bitfield(7, 7),
 *   parity = bitfield(4, 3), 
 *   baudrate = bitfield(2, 0), 
 * };
 * bitfieldWrite(UART0_CR, baudrate, 3);
 * bitfieldRead(UART0_CR, parity);
 * If your bitfields are only 1 bit wide, you can use the bitRead() and bitWrite() functions instead.
 * Based on : https://accu.org/journals/overload/13/68/goodliffe_281/
 */
#define bitfield(msb, lsb) (((msb - lsb + 1) << 4) | (lsb))
/**
 * @brief Macro to read a bit field from a register.
 * @param value The register value
 * @param bitfield The bit field to read (defined with bitfield() macro)
 */
#define bitfieldRead(value, bitfield) (((value) >> ((bitfield)&0x0f)) & ((1 << ((bitfield & 0xf0) >> 4)) - 1))
/**
 * @brief Macro to write a bit field to a register.
 * @param reg The register to write to
 * @param bitfield The bit field to write (defined with bitfield() macro)
 * @param value The value to write to the bit field
 */
#define bitfieldWrite(reg, bitfield, value) (reg = (reg & ~(((1 << ((bitfield & 0xf0) >> 4)) - 1) << (bitfield & 0x0f))) | (value << (bitfield & 0x0f)))