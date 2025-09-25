#include "../lib/led/led.h"
#include <unity.h>

void setUp(void) {
  led_init();
}

void tearDown(void) {
  HAL_GPIO_DeInit(LED_GPIO_PORT, LED_PIN);
}

void test_led_builtin_pin_number(void) {
  TEST_ASSERT_EQUAL(GPIO_PIN_5, LED_PIN);
}

void test_led_state_high(void) {
  led_on();
  HAL_Delay(1000);
  TEST_ASSERT_EQUAL(HIGH, led_state());
}

void test_led_state_low(void) {
  led_off();
  HAL_Delay(1000);
  TEST_ASSERT_EQUAL(LOW, led_state());
}

int main() {
  HAL_Init();      // initialize the HAL library
  HAL_Delay(2000); // service delay

  UNITY_BEGIN();
  RUN_TEST(test_led_builtin_pin_number);

  for (unsigned int i = 0; i < 5; i++) {
    RUN_TEST(test_led_state_high);
    HAL_Delay(500);
    RUN_TEST(test_led_state_low);
    HAL_Delay(500);
  }

  UNITY_END(); // stop unit testing

  while (1) {
  }
}

void SysTick_Handler(void) {
  HAL_IncTick();
}
