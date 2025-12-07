#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32l476xx.h"
#include "rtos.h"   // For I2C_Error, ms_since_start(), etc.

/*
 * Initialize I2C1 and I2C2 peripherals and GPIO pins.
 * - PB8 / PB9  used for I2C1 (SCL/SDA)  (AF4)
 * - PB10 / PB11 used for I2C2 (SCL/SDA) (AF4)
 */
void i2c_init(void);

/*
 * Blocking I2C write:
 *  - i2c:        I2C1 or I2C2
 *  - slave_addr: 7-bit slave address (unshifted, e.g., 0x41)
 *  - data:       pointer to bytes to send
 *  - data_len:   number of bytes to send
 *
 * Signature matches existing code: data is non-const.
 */
I2C_Error i2c_write(I2C_TypeDef* i2c,
                    uint8_t slave_addr,
                    uint8_t* data,
                    uint8_t data_len);

/*
 * Blocking I2C read:
 *  - i2c:        I2C1 or I2C2
 *  - slave_addr: 7-bit slave address (unshifted, e.g., 0x41)
 *  - data:       pointer to buffer to store received bytes
 *  - data_len:   number of bytes to read
 */
I2C_Error i2c_read(I2C_TypeDef* i2c,
                   uint8_t slave_addr,
                   uint8_t* data,
                   uint8_t data_len);

/*
 * Optional convenience helpers:
 * Write `data_len` bytes to a specific register on the slave.
 *   - First sends 1 byte (reg_addr), then data_len bytes.
 */
I2C_Error i2c_write_reg(I2C_TypeDef* i2c,
                        uint8_t slave_addr,
                        uint8_t reg_addr,
                        uint8_t* data,
                        uint8_t data_len);

/*
 * Read `data_len` bytes from a specific register on the slave.
 *   - First does a 1-byte write (reg_addr), then a read.
 */
I2C_Error i2c_read_reg(I2C_TypeDef* i2c,
                       uint8_t slave_addr,
                       uint8_t reg_addr,
                       uint8_t* data,
                       uint8_t data_len);

#endif /* I2C_H */