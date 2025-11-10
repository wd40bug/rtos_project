#include "I2C.c"
#include "gpio_exti.h"
#include "led.h"
#include "serial.h"
#include <stdbool.h>


int main(void) {
  HAL_Init();
  HAL_Delay(1000); // Allow time for monitor to connect

  I2C_init();
  serial_init();

  printf("Everything initialized!\n");

  // Test I2C read
  I2C_Read(0x50, NULL, 0);

  while (1) {
  }
}