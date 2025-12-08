#include "stm32l476xx.h"
#include "printf.h"
#include <stdbool.h>
void relays_init() {
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);

  GPIOA->MODER |= 1 << GPIO_MODER_MODE5_Pos;
  GPIOA->MODER |= 1 << GPIO_MODER_MODE6_Pos;
  GPIOA->MODER |= 1 << GPIO_MODER_MODE7_Pos;
}

void toggle_relay(uint8_t relay) {
  if (relay > 2) {
    return;
  }
  printf("Toggling relay: %d\n", relay);
  GPIOA->ODR ^= 1 << relay;
}
