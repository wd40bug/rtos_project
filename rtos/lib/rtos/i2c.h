#ifndef I2C_H
#define I2C_H

#include "rtos.h"
#include "stm32l476xx.h"


void i2c_init();
I2C_Error i2c_write(I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t* data, uint8_t data_len);
I2C_Error i2c_read(I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t* data, uint8_t data_len);


#endif /* ifndef I2C_H */
