#ifndef INA219_H
#define INA219_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"   // Change to your MCU family if different
#include <stdint.h>

// ===========================================
// INA219 I2C ADDRESSES (7-bit)
// ===========================================
// Example usage:
//   dev.address = INA219_ADDR_40;
//   dev.address = INA219_ADDR_41;
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
// DEFAULT CONFIG / CALIB (matches your sketch)
// ===========================================
#define INA219_DEFAULT_CONFIG   0x399FU
#define INA219_DEFAULT_CALIB    13400U

// ===========================================
// DEVICE STRUCT
// ===========================================
typedef struct {
    I2C_HandleTypeDef *hi2c;   // Pointer to I2C handle
    uint8_t address;           // 7-bit I2C address (e.g., 0x40, 0x41)

    float current_lsb;         // A/bit
    float power_lsb;           // W/bit
} INA219_Device;

// ===========================================
// PUBLIC API
// ===========================================

/**
 * @brief Initialize INA219 with default config/calibration (continuous mode).
 *        Uses the same config and calibration values as your Arduino code.
 *
 * @param dev        Pointer to INA219_Device instance
 * @param hi2c       Pointer to initialized I2C handle (HAL)
 * @param address7   7-bit I2C address (e.g., 0x40 or 0x41)
 *
 * @return HAL_OK on success, error status otherwise.
 */
HAL_StatusTypeDef INA219_Init(INA219_Device *dev,
                              I2C_HandleTypeDef *hi2c,
                              uint8_t address7);

/**
 * @brief Read shunt voltage in millivolts.
 *
 * @param dev       Pointer to INA219_Device
 * @param mV_out    Pointer to float to store the result in mV
 *
 * @return HAL_OK on success, error status otherwise.
 */
HAL_StatusTypeDef INA219_ReadShuntVoltage_mV(INA219_Device *dev, float *mV_out);

/**
 * @brief Read bus voltage in volts.
 *
 * @param dev       Pointer to INA219_Device
 * @param V_out     Pointer to float to store the result in V
 *
 * @return HAL_OK on success, error status otherwise.
 */
HAL_StatusTypeDef INA219_ReadBusVoltage_V(INA219_Device *dev, float *V_out);

/**
 * @brief Read current in milliamps.
 *
 * @param dev       Pointer to INA219_Device
 * @param mA_out    Pointer to float to store the result in mA
 *
 * @return HAL_OK on success, error status otherwise.
 */
HAL_StatusTypeDef INA219_ReadCurrent_mA(INA219_Device *dev, float *mA_out);

/**
 * @brief Read power in watts.
 *
 * @param dev       Pointer to INA219_Device
 * @param W_out     Pointer to float to store the result in W
 *
 * @return HAL_OK on success, error status otherwise.
 */
HAL_StatusTypeDef INA219_ReadPower_W(INA219_Device *dev, float *W_out);

/**
 * @brief Low-level: write 16-bit register.
 *
 * @param dev       Pointer to INA219_Device
 * @param reg       Register address
 * @param value     16-bit value to write
 *
 * @return HAL_OK on success, error status otherwise.
 */
HAL_StatusTypeDef INA219_WriteRegister(INA219_Device *dev,
                                       uint8_t reg,
                                       uint16_t value);

/**
 * @brief Low-level: read 16-bit register.
 *
 * @param dev       Pointer to INA219_Device
 * @param reg       Register address
 * @param value_out Pointer to 16-bit variable to store the value
 *
 * @return HAL_OK on success, error status otherwise.
 */
HAL_StatusTypeDef INA219_ReadRegister(INA219_Device *dev,
                                      uint8_t reg,
                                      uint16_t *value_out);

#ifdef __cplusplus
}
#endif

#endif // INA219_H
