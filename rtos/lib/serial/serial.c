#include "serial.h"
#include "common.h"
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_hal_uart.h"

static UART_HandleTypeDef UartHandle;

void serial_init() {
  GPIO_InitTypeDef GPIO_InitStruct;

  SERIAL_TX_GPIO_CLK_ENABLE();
  SERIAL_RX_GPIO_CLK_ENABLE();

  SERIAL_CLK_ENABLE();

  GPIO_InitStruct.Pin = SERIAL_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = SERIAL_TX_AF;

  HAL_GPIO_Init(SERIAL_TX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SERIAL_RX_PIN;
  GPIO_InitStruct.Alternate = SERIAL_RX_AF;

  HAL_GPIO_Init(SERIAL_RX_GPIO_PORT, &GPIO_InitStruct);
  UartHandle.Instance = SERIAL_UART;

  UartHandle.Init.BaudRate = 115200;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits = UART_STOPBITS_1;
  UartHandle.Init.Parity = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&UartHandle) != HAL_OK) {
    while (1) {
    }
  }
}

int serial_write(char* ptr, int len) {
  if (HAL_UART_Transmit(
      &UartHandle,
      (uint8_t*)ptr,
      len,
      0xFFFF
    ) == HAL_OK) {
    return len;
  }
  return 0;
}
