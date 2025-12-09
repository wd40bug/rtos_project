#include "i2c.h"

// redefine STM header variables as sanity check

#ifndef I2C_CR2_SADD_Pos
#define I2C_CR2_SADD_Pos      0U
#define I2C_CR2_SADD_Msk      (0x3FFU << I2C_CR2_SADD_Pos)
#endif

#ifndef I2C_CR2_NBYTES_Pos
#define I2C_CR2_NBYTES_Pos    16U
#define I2C_CR2_NBYTES_Msk    (0xFFU << I2C_CR2_NBYTES_Pos)
#endif

#ifndef I2C_CR2_RELOAD_Pos
#define I2C_CR2_RELOAD_Pos    24U
#define I2C_CR2_RELOAD        (1U << I2C_CR2_RELOAD_Pos)
#endif

#ifndef I2C_CR2_AUTOEND_Pos
#define I2C_CR2_AUTOEND_Pos   25U
#define I2C_CR2_AUTOEND       (1U << I2C_CR2_AUTOEND_Pos)
#endif

// ---- configuration macros ----

#define I2C_TIMEOUT_MS  10U           
#define I2C_TIMING_REG  0x30420F13U  

// ---- internal helpers ----

static bool i2c_wait_flag_set(I2C_TypeDef* i2c, uint32_t flag_mask)
{
    uint32_t start = (uint32_t)ms_since_start();
    while ((i2c->ISR & flag_mask) == 0U) {
        if (((uint32_t)ms_since_start() - start) > I2C_TIMEOUT_MS) {
            return false;
        }
    }
    return true;
}

static bool i2c_wait_flag_clear(I2C_TypeDef* i2c, uint32_t flag_mask)
{
    uint32_t start = (uint32_t)ms_since_start();
    while ((i2c->ISR & flag_mask) != 0U) {
        if (((uint32_t)ms_since_start() - start) > I2C_TIMEOUT_MS) {
            return false;
        }
    }
    return true;
}

static void i2c_clear_errors_and_stop(I2C_TypeDef* i2c)
{
    i2c->ICR = I2C_ICR_STOPCF    // Clear STOPF
             | I2C_ICR_NACKCF    // Clear NACKF
             | I2C_ICR_BERRCF    // Clear BERR
             | I2C_ICR_ARLOCF;   // Clear ARLO
}

static I2C_Error i2c_check_errors(I2C_TypeDef* i2c)
{
    uint32_t isr = i2c->ISR;

    if (isr & (I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_NACKF)) {
        i2c_clear_errors_and_stop(i2c);
        return INVALID_I2C;
    }

    return I2C_OK;
}

void i2c_init(void)
{
    // Enable GPIOB clock (PB8-11)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // Clear mode bits for PB8-11
    GPIOB->MODER &= ~(GPIO_MODER_MODE8_Msk | GPIO_MODER_MODE9_Msk);

    // Alternate function mode (10b)
    GPIOB->MODER |= (2U << GPIO_MODER_MODE8_Pos)
                  | (2U << GPIO_MODER_MODE9_Pos);

    // Open-drain, high speed
    GPIOB->OTYPER |= GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9;

    GPIOB->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED8_Pos)
                    | (3U << GPIO_OSPEEDR_OSPEED9_Pos);

    // AF4 for I2C on PB8-11
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL8_Msk | GPIO_AFRH_AFSEL9_Msk);
    GPIOB->AFR[1] |= (4U << GPIO_AFRH_AFSEL8_Pos)
                   | (4U << GPIO_AFRH_AFSEL9_Pos);

    // Disable I2C1 and I2C2 before config
    I2C1->CR1 &= ~I2C_CR1_PE;

    // Enable APB1 clocks
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    // Timing config
    I2C1->TIMINGR = I2C_TIMING_REG;

    // Enable peripherals
    I2C1->CR1 |= I2C_CR1_PE;
}

// Blocking write
I2C_Error i2c_write(I2C_TypeDef* i2c,
                    uint8_t slave_addr,
                    uint8_t* data,
                    uint8_t data_len)
{
    if (data_len == 0U) {
        return I2C_OK;
    }

    // Wait until bus is free
    if (!i2c_wait_flag_clear(i2c, I2C_ISR_BUSY)) {
        return INVALID_I2C;
    }

    i2c_clear_errors_and_stop(i2c);

    // Configure CR2
    uint32_t cr2 = 0;
    cr2 |= (((uint32_t)slave_addr << 1) << I2C_CR2_SADD_Pos);   // 7-bit address
    cr2 |= ((uint32_t)data_len  << I2C_CR2_NBYTES_Pos);  // NBYTES
    cr2 |= I2C_CR2_START;                                // START
    cr2 |= I2C_CR2_AUTOEND;                              // AUTOEND
    // RD_WRN = 0 -> write
    i2c->CR2 = cr2;

    // Send bytes
    for (uint8_t i = 0; i < data_len; i++) {
        while ((i2c->ISR & I2C_ISR_TXE) == 0U) {
            I2C_Error err = i2c_check_errors(i2c);
            if (err != I2C_OK) {
               return err;
            }
        }
        i2c->TXDR = data[i];
    }

    // Wait for STOPF
    if (!i2c_wait_flag_set(i2c, I2C_ISR_STOPF)) {
        return INVALID_I2C;
    }

    i2c->ICR = I2C_ICR_STOPCF;
    return I2C_OK;
}

// Blocking read
I2C_Error i2c_read(I2C_TypeDef* i2c,
                   uint8_t slave_addr,
                   uint8_t* data,
                   uint8_t data_len)
{
    if (data_len == 0U) {
        return I2C_OK;
    }

    if (!i2c_wait_flag_clear(i2c, I2C_ISR_BUSY)) {
        return INVALID_I2C;
    }

    i2c_clear_errors_and_stop(i2c);

    uint32_t cr2 = 0;
    cr2 |= (((uint32_t)slave_addr << 1) << I2C_CR2_SADD_Pos);;
    cr2 |= ((uint32_t)data_len  << I2C_CR2_NBYTES_Pos);
    cr2 |= I2C_CR2_START;
    cr2 |= I2C_CR2_AUTOEND;
    cr2 |= I2C_CR2_RD_WRN;   // read
    i2c->CR2 = cr2;

    for (uint8_t i = 0; i < data_len; i++) {
        while ((i2c->ISR & I2C_ISR_RXNE) == 0U) {
            I2C_Error err = i2c_check_errors(i2c);
            if (err != I2C_OK) {
                return err;
            }
        }
        data[i] = (uint8_t)(i2c->RXDR & 0xFFU);
    }

    if (!i2c_wait_flag_set(i2c, I2C_ISR_STOPF)) {
        return INVALID_I2C;
    }

    i2c->ICR = I2C_ICR_STOPCF;
    return I2C_OK;
}
