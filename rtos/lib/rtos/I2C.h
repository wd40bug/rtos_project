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
 *
 * This sets up:
 *  - GPIO alternate function
 *  - I2C timing registers
 *  - Enables the peripherals
 */
void i2c_init(void);

/*
 * Blocking I2C write:
 *  - i2c:        I2C1 or I2C2
 *  - slave_addr: 7-bit slave address (unshifted, e.g., 0x41)
 *  - data:       pointer to bytes to send
 *  - data_len:   number of bytes to send
 *
 * Returns:
 *   I2C_OK       on success
 *   INVALID_I2C  on NACK / bus error / timeout
 */
I2C_Error i2c_write(I2C_TypeDef* i2c,
                    uint8_t slave_addr,
                    const uint8_t* data,
                    uint8_t data_len);

/*
 * Blocking I2C read:
 *  - i2c:        I2C1 or I2C2
 *  - slave_addr: 7-bit slave address (unshifted, e.g., 0x41)
 *  - data:       pointer to buffer where received bytes will be stored
 *  - data_len:   number of bytes to read
 *
 * Returns:
 *   I2C_OK       on success
 *   INVALID_I2C  on NACK / bus error / timeout
 */
I2C_Error i2c_read(I2C_TypeDef* i2c,
                   uint8_t slave_addr,
                   uint8_t* data,
                   uint8_t data_len);

/*
 * Optional convenience helpers (you can use or ignore):
 *
 * Write `data_len` bytes to a specific register on the slave.
 *   - First sends 1 byte (reg_addr), then data_len bytes.
 *   - Performed as a single write transaction (STOP at end).
 */
I2C_Error i2c_write_reg(I2C_TypeDef* i2c,
                        uint8_t slave_addr,
                        uint8_t reg_addr,
                        const uint8_t* data,
                        uint8_t data_len);

/*
 * Read `data_len` bytes from a specific register on the slave.
 *   - First does a small write to set the register pointer (reg_addr),
 *     then a separate read transaction.
 *   - Some devices require repeated-start instead of STOP+START; for those,
 *     you’d adjust this logic, but this works for many common sensors.
 */
I2C_Error i2c_read_reg(I2C_TypeDef* i2c,
                       uint8_t slave_addr,
                       uint8_t reg_addr,
                       uint8_t* data,
                       uint8_t data_len);

#endif /* I2C_H */
