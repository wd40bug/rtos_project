#include "../lib/gpio_exti/gpio_exti.h"
#include <unity.h>

void setUp() {
  __HAL_RCC_GPIOF_CLK_ENABLE();
}

void tearDown() {}

static bool callback_run;

static void callback(void) {
  callback_run = true;
}

void test_pin_interrupt(uint16_t pin) {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  callback_run = false;
  GPIO_EXTI_Err_t err = gpio_exti_register_callback(pin, callback, true);
  TEST_ASSERT_EQUAL(GPIO_EXTI_ERR_OK, err);
  __HAL_GPIO_EXTI_GENERATE_SWIT(pin);
  HAL_Delay(500);
}

#define test_pin_interrupt_n(n)                                                \
  void test_pin_interrupt##n(void) {                                           \
    test_pin_interrupt(1 << n);                                                \
    TEST_ASSERT_TRUE(callback_run);                                            \
  }

test_pin_interrupt_n(0);
test_pin_interrupt_n(1);
test_pin_interrupt_n(2);
test_pin_interrupt_n(3);
test_pin_interrupt_n(4);
test_pin_interrupt_n(5);
test_pin_interrupt_n(6);
test_pin_interrupt_n(7);
test_pin_interrupt_n(8);
test_pin_interrupt_n(9);
test_pin_interrupt_n(10);
test_pin_interrupt_n(11);
test_pin_interrupt_n(12);
test_pin_interrupt_n(13);
test_pin_interrupt_n(14);
test_pin_interrupt_n(15);

int main() {
  HAL_Init();      // initialize the HAL library
  HAL_Delay(2000); // service delay

  UNITY_BEGIN();
  RUN_TEST(test_pin_interrupt15);
  RUN_TEST(test_pin_interrupt14);
  RUN_TEST(test_pin_interrupt13);
  RUN_TEST(test_pin_interrupt12);
  RUN_TEST(test_pin_interrupt11);
  RUN_TEST(test_pin_interrupt10);
  RUN_TEST(test_pin_interrupt9);
  RUN_TEST(test_pin_interrupt8);
  RUN_TEST(test_pin_interrupt7);
  RUN_TEST(test_pin_interrupt6);
  RUN_TEST(test_pin_interrupt5);
  RUN_TEST(test_pin_interrupt4);
  RUN_TEST(test_pin_interrupt3);
  RUN_TEST(test_pin_interrupt2);
  RUN_TEST(test_pin_interrupt1);
  RUN_TEST(test_pin_interrupt0);
  UNITY_END(); // stop unit testing

  while (1) {
  }
}

void SysTick_Handler(void) {
  HAL_IncTick();
}
