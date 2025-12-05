#include "i2c.h"
#include "rtos.h"
#include "scheduling.h"
#include "stm32l476xx.h"

typedef struct {
  bool is_read;
  uint8_t slave_addr;
  uint8_t data_len;
  uint8_t* data;
  // Read
  TASK_HANDLE handle;
} I2C_Action;

#define QUEUE_CAPACITY 100
#define QUEUE_TYPENAME I2C_Queue
#define QUEUE_TYPE I2C_Action
#define QUEUE_NO_CONTAINS
#include "queue.h"

void i2c_init() {
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

  GPIOB->MODER &=
      ~(GPIO_MODER_MODE8 | GPIO_MODER_MODE9 | GPIO_MODER_MODE10 |
        GPIO_MODER_MODE11);
  GPIOB->MODER |= 2 << GPIO_MODER_MODE8_Pos;
  GPIOB->MODER |= 2 << GPIO_MODER_MODE9_Pos;
  GPIOB->MODER |= 2 << GPIO_MODER_MODE10_Pos;
  GPIOB->MODER |= 2 << GPIO_MODER_MODE11_Pos;

  GPIOB->AFR[1] |= 4 << GPIO_AFRH_AFSEL8_Pos;
  GPIOB->AFR[1] |= 4 << GPIO_AFRH_AFSEL9_Pos;
  GPIOB->AFR[1] |= 4 << GPIO_AFRH_AFSEL10_Pos;
  GPIOB->AFR[1] |= 4 << GPIO_AFRH_AFSEL11_Pos;

  I2C1->CR1 &= ~I2C_CR1_PE;
  I2C2->CR1 &= ~I2C_CR1_PE;

  RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
  RCC->APB1ENR1 |= RCC_APB1ENR1_I2C2EN;

  I2C1->TIMINGR = 0x30420F13;
  I2C2->TIMINGR = 0x30420F13;

  I2C1->CR1 |= I2C_CR1_PE;
  I2C2->CR1 |= I2C_CR1_PE;
}

// Chats attempt
// I2C_Error i2c_write(
//     I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t* data, uint8_t data_len
// ) {
//     // Clear any old errors
//     i2c->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF | I2C_ICR_BERRCF | I2C_ICR_ARLOCF;

//     // Configure transfer
//     i2c->CR2 =
//         (slave_addr << I2C_CR2_SADD_Pos) |
//         (data_len << I2C_CR2_NBYTES_Pos) |
//         I2C_CR2_START |
//         I2C_CR2_AUTOEND;

//     for (size_t i = 0; i < data_len; i++) {
//         // Wait for TXIS = ready to write next byte
//         while (!(i2c->ISR & I2C_ISR_TXIS));

//         i2c->TXDR = data[i];
//     }

//     // Wait for STOPF
//     while (!(i2c->ISR & I2C_ISR_STOPF));

//     return I2C_OK;
// }

// My 1st attempt
// I2C_Error i2c_write( // Skipping the last one entirely
//     I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t* data, uint8_t data_len
// ) {
//   i2c->ICR |= I2C_ICR_STOPCF | I2C_ICR_NACKCF;
//   //size_t index = 0;
//   // i2c->TXDR = data[index++];
//   i2c->CR2 = data_len << I2C_CR2_NBYTES_Pos | slave_addr << 1 |
//              I2C_CR2_START | I2C_CR2_AUTOEND;
//   for (size_t index = 0 ; index < data_len ; index++) { // while (index < data_len) 
//     while (!(i2c->ISR & I2C_ISR_TXIS)){ // Supposed to wait for TXIS and not TXE?
//         if (i2c->ISR & I2C_ISR_NACKF) return -1;
//     }
//     i2c->TXDR = data[index];
//   }

//   while (!(i2c->ISR & I2C_ISR_STOPF));

//   return I2C_OK;
// }

// Wills old code
// I2C_Error i2c_write(
//     I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t* data, uint8_t data_len
// ) {
//   i2c->ICR |= I2C_ICR_STOPCF | I2C_ICR_NACKCF;
//   size_t index = 0;
//   // i2c->TXDR = data[index++];
//   i2c->CR2 = data_len << I2C_CR2_NBYTES_Pos | slave_addr << 1 |
//              I2C_CR2_START | I2C_CR2_AUTOEND;
//   while (index < data_len) {
//     while (!(i2c->ISR & I2C_ISR_TXE))
//       ;
//     i2c->TXDR = data[index++];
//   }
//   return I2C_OK;
// }

I2C_Error i2c_read(
    I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t* data, uint8_t data_len
) {
  i2c->CR2 = data_len << I2C_CR2_NBYTES_Pos | slave_addr << I2C_CR2_SADD_Pos |
             I2C_CR2_START | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN;
  size_t index = 0;
  while (index < data_len) {
    while (!(i2c->ISR & I2C_ISR_TXE))
      ;
    i2c->TXDR = data[index++];
  }
  return I2C_OK;
}
