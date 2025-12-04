#ifndef INA219_H
#define INA219_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32l476xx.h"   // CMSIS device header for STM32L476

// ===========================================
// INA219 I2C ADDRESSES (7-bit)
// ===========================================
#define INA219_ADDR_40   0x40U
#define INA219_ADDR_41   0x41U

// ===========================================
// INA219 REGISTERS
// ===========================================
#define INA219_REG_CONFIG    0x00U
#define INA219_REG_SHUNT     0x01U
#define INA219_REG_BUS       0x02U
#define INA219_REG_POWER     0x03U
#define INA219_REG_CURRENT   0x04U
#define INA219_REG_CALIB     0x05U

// ===========================================
// DEFAULT CONFIG / CALIB (matches Arduino sketch)
// ===========================================
#define INA219_DEFAULT_CONFIG   0x399FU
#define INA219_DEFAULT_CALIB    13400U

// ===========================================
// STATUS ENUM
// ===========================================
typedef enum {
    INA219_OK = 0,
    INA219_ERROR_PARAM,
    INA219_ERROR_TIMEOUT,
    INA219_ERROR_NACK,
    INA219_ERROR_BUS
} INA219_Status;

// ===========================================
// DEVICE HANDLE
// ===========================================
typedef struct {
    I2C_TypeDef *I2C;       // Pointer to I2C peripheral (e.g., I2C1, I2C2)
    uint8_t      address;   // 7-bit I2C address (0x40, 0x41, etc.)

    float current_lsb;      // A/bit
    float power_lsb;        // W/bit
} INA219_Device;

// ===========================================
// PUBLIC API
// ===========================================

/**
 * @brief Initialize INA219 with default config/calibration (continuous mode).
 *        The I2C peripheral must already be configured and enabled.
 *
 * @param dev        Pointer to INA219_Device instance
 * @param I2Cx       Pointer to I2C instance (I2C1, I2C2, ...)
 * @param address7   7-bit I2C address (e.g., 0x40 or 0x41)
 *
 * @return INA219_OK on success, error code otherwise.
 */
INA219_Status INA219_Init(INA219_Device *dev,
                          I2C_TypeDef *I2Cx,
                          uint8_t address7);

/**
 * @brief Read shunt voltage in millivolts.
 */
INA219_Status INA219_ReadShuntVoltage_mV(INA219_Device *dev, float *mV_out);

/**
 * @brief Read bus voltage in volts.
 */
INA219_Status INA219_ReadBusVoltage_V(INA219_Device *dev, float *V_out);

/**
 * @brief Read current in milliamps.
 */
INA219_Status INA219_ReadCurrent_mA(INA219_Device *dev, float *mA_out);

/**
 * @brief Read power in watts.
 */
INA219_Status INA219_ReadPower_W(INA219_Device *dev, float *W_out);

/**
 * @brief Low-level: write 16-bit register.
 */
INA219_Status INA219_WriteRegister(INA219_Device *dev,
                                   uint8_t reg,
                                   uint16_t value);

/**
 * @brief Low-level: read 16-bit register.
 */
INA219_Status INA219_ReadRegister(INA219_Device *dev,
                                  uint8_t reg,
                                  uint16_t *value_out);

#ifdef __cplusplus
}
#endif

#endif
