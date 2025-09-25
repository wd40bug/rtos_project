#include "led.h"
#include "common.h"
#include "stm32l4xx_hal_gpio.h"

void led_init() {
  LED_GPIO_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
}

void led_on() {
  HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, HIGH);
}

void led_off() {
  HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, LOW);
}

void led_toggle() {
  HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
}

Signal led_state() {
  return HAL_GPIO_ReadPin(LED_GPIO_PORT, LED_PIN);
}
