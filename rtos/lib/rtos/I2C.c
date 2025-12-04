#include "I2C.h"

// Simple loop-based timeout (no HAL, no RTOS assumed)
#ifndef INA219_I2C_TIMEOUT_LOOPS
#define INA219_I2C_TIMEOUT_LOOPS  100000UL
#endif

// ===========================================
// INTERNAL HELPERS
// ===========================================

static INA219_Status INA219_I2C_WriteBytes(INA219_Device *dev,
                                           const uint8_t *data,
                                           uint8_t length);

static INA219_Status INA219_I2C_ReadBytes(INA219_Device *dev,
                                          uint8_t *data,
                                          uint8_t length);

// ===========================================
// LOW-LEVEL REGISTER ACCESS
// ===========================================

INA219_Status INA219_WriteRegister(INA219_Device *dev,
                                   uint8_t reg,
                                   uint16_t value)
{
    if (dev == NULL) {
        return INA219_ERROR_PARAM;
    }

    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (uint8_t)(value >> 8);   // MSB
    buf[2] = (uint8_t)(value & 0xFF); // LSB

    return INA219_I2C_WriteBytes(dev, buf, 3);
}

INA219_Status INA219_ReadRegister(INA219_Device *dev,
                                  uint8_t reg,
                                  uint16_t *value_out)
{
    if (dev == NULL || value_out == NULL) {
        return INA219_ERROR_PARAM;
    }

    INA219_Status status;
    uint8_t buf[2];

    // First, write the register address
    status = INA219_I2C_WriteBytes(dev, &reg, 1);
    if (status != INA219_OK) {
        return status;
    }

    // Then, read 2 bytes
    status = INA219_I2C_ReadBytes(dev, buf, 2);
    if (status != INA219_OK) {
        return status;
    }

    *value_out = (uint16_t)((buf[0] << 8) | buf[1]);
    return INA219_OK;
}

// ===========================================
// INITIALIZATION
// ===========================================

INA219_Status INA219_Init(INA219_Device *dev,
                          I2C_TypeDef *I2Cx,
                          uint8_t address7)
{
    if (dev == NULL || I2Cx == NULL) {
        return INA219_ERROR_PARAM;
    }

    dev->I2C     = I2Cx;
    dev->address = address7;

    // Your original calibration:
    //   Current_LSB = 30.5 µA = 0.0000305 A/bit
    //   Power_LSB   = 20 * Current_LSB = 0.00061 W/bit
    dev->current_lsb = 0.0000305f;
    dev->power_lsb   = dev->current_lsb * 20.0f;

    INA219_Status status;

    status = INA219_WriteRegister(dev, INA219_REG_CONFIG, INA219_DEFAULT_CONFIG);
    if (status != INA219_OK) {
        return status;
    }

    status = INA219_WriteRegister(dev, INA219_REG_CALIB, INA219_DEFAULT_CALIB);
    if (status != INA219_OK) {
        return status;
    }

    return INA219_OK;
}

// ===========================================
// HIGH-LEVEL MEASUREMENT FUNCTIONS
// ===========================================

INA219_Status INA219_ReadShuntVoltage_mV(INA219_Device *dev, float *mV_out)
{
    if (dev == NULL || mV_out == NULL) {
        return INA219_ERROR_PARAM;
    }

    uint16_t raw;
    INA219_Status status = INA219_ReadRegister(dev, INA219_REG_SHUNT, &raw);
    if (status != INA219_OK) {
        return status;
    }

    // Shunt voltage is signed, 10 µV per bit -> 0.01 mV per bit
    int16_t signed_raw = (int16_t)raw;
    *mV_out = (float)signed_raw * 0.01f;

    return INA219_OK;
}

INA219_Status INA219_ReadBusVoltage_V(INA219_Device *dev, float *V_out)
{
    if (dev == NULL || V_out == NULL) {
        return INA219_ERROR_PARAM;
    }

    uint16_t raw;
    INA219_Status status = INA219_ReadRegister(dev, INA219_REG_BUS, &raw);
    if (status != INA219_OK) {
        return status;
    }

    // Bits [2:0] are flags; voltage data is in bits [15:3]
    raw >>= 3;
    // 4 mV per bit
    *V_out = (float)raw * 0.004f;

    return INA219_OK;
}

INA219_Status INA219_ReadCurrent_mA(INA219_Device *dev, float *mA_out)
{
    if (dev == NULL || mA_out == NULL) {
        return INA219_ERROR_PARAM;
    }

    uint16_t raw;
    INA219_Status status = INA219_ReadRegister(dev, INA219_REG_CURRENT, &raw);
    if (status != INA219_OK) {
        return status;
    }

    int16_t signed_raw = (int16_t)raw;
    float current_A = (float)signed_raw * dev->current_lsb;
    *mA_out = current_A * 1000.0f;

    return INA219_OK;
}

INA219_Status INA219_ReadPower_W(INA219_Device *dev, float *W_out)
{
    if (dev == NULL || W_out == NULL) {
        return INA219_ERROR_PARAM;
    }

    uint16_t raw;
    INA219_Status status = INA219_ReadRegister(dev, INA219_REG_POWER, &raw);
    if (status != INA219_OK) {
        return status;
    }

    *W_out = (float)raw * dev->power_lsb;
    return INA219_OK;
}

// ===========================================
// CMSIS I2C IMPLEMENTATION (blocking, polled)
// ===========================================

static INA219_Status INA219_I2C_WaitFlagSet(I2C_TypeDef *I2C, uint32_t flag)
{
    uint32_t timeout = INA219_I2C_TIMEOUT_LOOPS;
    while ((I2C->ISR & flag) == 0U) {
        if ((I2C->ISR & I2C_ISR_NACKF) != 0U) {
            // Clear NACK flag
            I2C->ICR = I2C_ICR_NACKCF;
            return INA219_ERROR_NACK;
        }
        if (--timeout == 0U) {
            return INA219_ERROR_TIMEOUT;
        }
    }
    return INA219_OK;
}

static INA219_Status INA219_I2C_WaitFlagClear(I2C_TypeDef *I2C, uint32_t flag)
{
    uint32_t timeout = INA219_I2C_TIMEOUT_LOOPS;
    while ((I2C->ISR & flag) != 0U) {
        if (--timeout == 0U) {
            return INA219_ERROR_TIMEOUT;
        }
    }
    return INA219_OK;
}

static INA219_Status INA219_I2C_WriteBytes(INA219_Device *dev,
                                           const uint8_t *data,
                                           uint8_t length)
{
    if (dev == NULL || data == NULL || length == 0U) {
        return INA219_ERROR_PARAM;
    }

    I2C_TypeDef *I2C = dev->I2C;

    // Wait until not busy
    INA219_Status status = INA219_I2C_WaitFlagClear(I2C, I2C_ISR_BUSY);
    if (status != INA219_OK) {
        return status;
    }

    // Clear STOP/NACK flags
    I2C->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF;

    uint32_t devaddr = ((uint32_t)dev->address << 1); // 7-bit addr shifted for SADD

    // Configure CR2: slave address, write, number of bytes, autoend, start
    uint32_t cr2 = I2C->CR2;
    cr2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_NBYTES_Msk |
             I2C_CR2_RD_WRN    | I2C_CR2_START     | I2C_CR2_STOP | I2C_CR2_AUTOEND);

    cr2 |= (devaddr & I2C_CR2_SADD_Msk); // SADD[9:0]
    cr2 |= ((uint32_t)length << I2C_CR2_NBYTES_Pos);
    cr2 |= I2C_CR2_AUTOEND;             // automatic STOP at end
    // Write mode: RD_WRN = 0
    cr2 |= I2C_CR2_START;               // generate START

    I2C->CR2 = cr2;

    // Transmit bytes
    for (uint8_t i = 0; i < length; ++i) {
        status = INA219_I2C_WaitFlagSet(I2C, I2C_ISR_TXIS);
        if (status != INA219_OK) {
            return status;
        }
        I2C->TXDR = data[i];
    }

    // Wait for STOP
    status = INA219_I2C_WaitFlagSet(I2C, I2C_ISR_STOPF);
    if (status != INA219_OK) {
        return status;
    }

    // Clear STOP flag
    I2C->ICR = I2C_ICR_STOPCF;

    return INA219_OK;
}

static INA219_Status INA219_I2C_ReadBytes(INA219_Device *dev,
                                          uint8_t *data,
                                          uint8_t length)
{
    if (dev == NULL || data == NULL || length == 0U) {
        return INA219_ERROR_PARAM;
    }

    I2C_TypeDef *I2C = dev->I2C;

    // Wait until not busy
    INA219_Status status = INA219_I2C_WaitFlagClear(I2C, I2C_ISR_BUSY);
    if (status != INA219_OK) {
        return status;
    }

    // Clear STOP/NACK flags
    I2C->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF;

    uint32_t devaddr = ((uint32_t)dev->address << 1);

    // Configure CR2: slave address, read, number of bytes, autoend, start
    uint32_t cr2 = I2C->CR2;
    cr2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_NBYTES_Msk |
             I2C_CR2_RD_WRN    | I2C_CR2_START     | I2C_CR2_STOP | I2C_CR2_AUTOEND);

    cr2 |= (devaddr & I2C_CR2_SADD_Msk);
    cr2 |= ((uint32_t)length << I2C_CR2_NBYTES_Pos);
    cr2 |= I2C_CR2_RD_WRN;   // read mode
    cr2 |= I2C_CR2_AUTOEND;
    cr2 |= I2C_CR2_START;

    I2C->CR2 = cr2;

    // Receive bytes
    for (uint8_t i = 0; i < length; ++i) {
        status = INA219_I2C_WaitFlagSet(I2C, I2C_ISR_RXNE);
        if (status != INA219_OK) {
            return status;
        }
        data[i] = (uint8_t)(I2C->RXDR & 0xFFU);
    }

    // Wait for STOP
    status = INA219_I2C_WaitFlagSet(I2C, I2C_ISR_STOPF);
    if (status != INA219_OK) {
        return status;
    }

    // Clear STOP flag
    I2C->ICR = I2C_ICR_STOPCF;

    return INA219_OK;
}
