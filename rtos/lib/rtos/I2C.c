#include "I2C.h"

// Internal timeout for HAL I2C calls (ms)
#ifndef INA219_I2C_TIMEOUT
#define INA219_I2C_TIMEOUT  10U
#endif

// ===========================================
// LOW-LEVEL REGISTER ACCESS
// ===========================================

HAL_StatusTypeDef INA219_WriteRegister(INA219_Device *dev,
                                       uint8_t reg,
                                       uint16_t value)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(value >> 8);   // MSB first
    buf[1] = (uint8_t)(value & 0xFF); // LSB

    // HAL expects 8-bit address (7-bit << 1)
    uint16_t dev_addr = (uint16_t)(dev->address << 1);

    return HAL_I2C_Mem_Write(dev->hi2c,
                             dev_addr,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             buf,
                             2,
                             INA219_I2C_TIMEOUT);
}

HAL_StatusTypeDef INA219_ReadRegister(INA219_Device *dev,
                                      uint8_t reg,
                                      uint16_t *value_out)
{
    uint8_t buf[2];
    uint16_t dev_addr = (uint16_t)(dev->address << 1);

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(dev->hi2c,
                                               dev_addr,
                                               reg,
                                               I2C_MEMADD_SIZE_8BIT,
                                               buf,
                                               2,
                                               INA219_I2C_TIMEOUT);
    if (status != HAL_OK) {
        return status;
    }

    *value_out = (uint16_t)((buf[0] << 8) | buf[1]);
    return HAL_OK;
}

// ===========================================
// INITIALIZATION
// ===========================================

HAL_StatusTypeDef INA219_Init(INA219_Device *dev,
                              I2C_HandleTypeDef *hi2c,
                              uint8_t address7)
{
    if (dev == NULL || hi2c == NULL) {
        return HAL_ERROR;
    }

    dev->hi2c    = hi2c;
    dev->address = address7;

    // Using your Arduino-based calibration:
    //   Current_LSB = 30.5 uA = 0.0000305 A/bit
    //   Power_LSB   = 20 * Current_LSB = 0.00061 W/bit
    dev->current_lsb = 0.0000305f;
    dev->power_lsb   = dev->current_lsb * 20.0f;

    HAL_StatusTypeDef status;

    // Write CONFIG register
    status = INA219_WriteRegister(dev, INA219_REG_CONFIG, INA219_DEFAULT_CONFIG);
    if (status != HAL_OK) {
        return status;
    }

    // Write CALIBRATION register
    status = INA219_WriteRegister(dev, INA219_REG_CALIB, INA219_DEFAULT_CALIB);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_OK;
}

// ===========================================
// HIGH-LEVEL MEASUREMENT FUNCTIONS
// ===========================================

HAL_StatusTypeDef INA219_ReadShuntVoltage_mV(INA219_Device *dev, float *mV_out)
{
    if (dev == NULL || mV_out == NULL) {
        return HAL_ERROR;
    }

    uint16_t raw;
    HAL_StatusTypeDef status = INA219_ReadRegister(dev, INA219_REG_SHUNT, &raw);
    if (status != HAL_OK) {
        return status;
    }

    // Shunt voltage is signed (two's complement), 10 µV per bit -> 0.01 mV per bit
    int16_t signed_raw = (int16_t)raw;
    *mV_out = (float)signed_raw * 0.01f;

    return HAL_OK;
}

HAL_StatusTypeDef INA219_ReadBusVoltage_V(INA219_Device *dev, float *V_out)
{
    if (dev == NULL || V_out == NULL) {
        return HAL_ERROR;
    }

    uint16_t raw;
    HAL_StatusTypeDef status = INA219_ReadRegister(dev, INA219_REG_BUS, &raw);
    if (status != HAL_OK) {
        return status;
    }

    // Bits [2:0] are flags; voltage data is in bits [15:3]
    raw >>= 3;
    // 4 mV per bit
    *V_out = (float)raw * 0.004f;

    return HAL_OK;
}

HAL_StatusTypeDef INA219_ReadCurrent_mA(INA219_Device *dev, float *mA_out)
{
    if (dev == NULL || mA_out == NULL) {
        return HAL_ERROR;
    }

    uint16_t raw;
    HAL_StatusTypeDef status = INA219_ReadRegister(dev, INA219_REG_CURRENT, &raw);
    if (status != HAL_OK) {
        return status;
    }

    // Current register is signed
    int16_t signed_raw = (int16_t)raw;
    // Convert to Amps using LSB, then to mA
    float current_A = (float)signed_raw * dev->current_lsb;
    *mA_out = current_A * 1000.0f;

    return HAL_OK;
}

HAL_StatusTypeDef INA219_ReadPower_W(INA219_Device *dev, float *W_out)
{
    if (dev == NULL || W_out == NULL) {
        return HAL_ERROR;
    }

    uint16_t raw;
    HAL_StatusTypeDef status = INA219_ReadRegister(dev, INA219_REG_POWER, &raw);
    if (status != HAL_OK) {
        return status;
    }

    // Power register is unsigned
    *W_out = (float)raw * dev->power_lsb;

    return HAL_OK;
}
