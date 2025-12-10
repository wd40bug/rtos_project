#ifndef I2C_H
#define I2C_H

#include "stm32l476xx.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  I2C_OK,
  I2C_QUEUE_FULL,
  INVALID_I2C,
  CONCURRENT_RECV,
} I2C_Error;
/*
 * Initialize I2C1 and I2C2 peripherals and GPIO pins.
 * - PB8 / PB9  used for I2C1 (SCL/SDA)  (AF4)
 * - PB10 / PB11 used for I2C2 (SCL/SDA) (AF4)
 */
void i2c_init(void);
I2C_Error i2c_write(uint8_t slave_addr, uint8_t* data, uint8_t data_len);

I2C_Error i2c_read(uint8_t slave_addr, uint8_t* data, uint8_t data_len);

/*
 * Optional convenience helpers:
 * Write `data_len` bytes to a specific register on the slave.
 *   - First sends 1 byte (reg_addr), then data_len bytes.
 */
I2C_Error i2c_write_reg(
    uint8_t slave_addr, uint8_t reg_addr, uint16_t data
);

/*
 * Read `data_len` bytes from a specific register on the slave.
 *   - First does a 1-byte write (reg_addr), then a read.
 */
I2C_Error i2c_read_reg(
    uint8_t slave_addr, uint8_t reg_addr, uint16_t* data
);

#endif /* I2C_H */
