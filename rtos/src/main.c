#include "button.h"
#include "gpio_exti.h"
#include "led.h"
#include "serial.h"
#include <stdbool.h>

void toggle_on(void) {
  led_on();
}

void toggle_off(void) {
  led_off();
}

int main(void) {
  HAL_Init();
  HAL_Delay(1000); // Allow time for monitor to connect

  serial_init();
  led_init();
  button_init();

  printf("Everything initialized!\n");

  // gpio_exti_register_callback(BUTTON_PIN, toggle_led, false);
  button_on_pressed(toggle_on, BUTTON_OVERRIDE_NONE);
  button_on_released(toggle_off, BUTTON_OVERRIDE_NONE);

  while (1) {
  }
}

// Must be present for serial to work
int _write(int file, char* ptr, int len) {
  return serial_write(ptr, len);
}

// Must be present for HAL_Delay to work
// Runs every 1ms
void SysTick_Handler(void) {
  HAL_IncTick();
}
