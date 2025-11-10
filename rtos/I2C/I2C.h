#ifndef I2C_H
#define I2C_H

#define I2C_INSTANCE I2C1
#define I2C_CLK_ENABLE() __HAL_RCC_I2C1_CLK_ENABLE()
#define I2C_SCL_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2C_SDA_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define I2C_SCL_PIN GPIO_PIN_8
#define I2C_SCL_GPIO_PORT GPIOB
#define I2C_SCL_AF GPIO_AF4_I2C1

#define I2C_SDA_PIN GPIO_PIN_9
#define I2C_SDA_GPIO_PORT GPIOB
#define I2C_SDA_AF GPIO_AF4_I2C1

/**
 * @brief Initalize the I2C peripheral
 */
void I2C_init();

void I2C_Write(uint16_t DevAddress, uint8_t* pData, uint16_t Size);
uint8_t I2C_Read(uint16_t DevAddress, uint8_t* pData, uint16_t Size);


#endif