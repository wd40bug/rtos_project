#include "common.h"
#include "stm32l4xx_hal_def.h"

static I2C_HandleTypeDef hi2c1;

void I2C_init() {
    GPIO_initTypeDef GPIO_InitStruct;

    I2C_CLK_ENABLE();
    I2C_SCL_GPIO_CLK_ENABLE();
    I2C_SDA_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Pin = I2C_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = I2C_SCL_AF;

    HAL_GPIO_Init(I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = I2C_SDA_PIN;
    GPIO.InitStruct.Mode = GPIO_MODE_AF_OD;


    HAL_GPIO_Init(I2C_SDA_GPIO_PORT, &GPIO_InitStruct);
    hi2c1.Instance = I2C_INSTANCE;

    hi2c1.Init.Timing = 0x00707CBB; // 100kHz @ 80MHz
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        while (1) {
        }
    }
}

int I2C_write(uint16_t DevAddress, uint8_t* pData, uint16_t Size) {
    if (HAL_I2C_Master_Transmit( &hi2c1, DevAddress, pData, Size, 0xFFFF) == HAL_OK) {
        return Size;
    }
    return 0;
}

int I2C_read(uint16_t DevAddress, uint8_t* pData, uint16_t Size) {
    if (HAL_I2C_Master_Receive( &hi2c1, DevAddress, pData, Size, 0xFFFF) == HAL_OK) {
        return Size;
    }
    return 0;
}

