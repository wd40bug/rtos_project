#include "gpio_exti.h"

void default_callback(void) {
  // NOTE: Could add debug printing here :)
  __NOP();
}

typedef enum {
  IRQ_0,
  IRQ_1,
  IRQ_2,
  IRQ_3,
  IRQ_4,
  IRQ_9_5,
  IRQ_15_10,
  INVALID,
} pin_irq;

// Won't do anything

typedef struct {
  GPIO_EXTI_Callback callback;
  uint16_t pin;
} InterruptInfo;

#define NO_PIN 0

static InterruptInfo interrupts[INVALID] = {
    [IRQ_0] = {.callback = default_callback, .pin = NO_PIN},
    [IRQ_1] = {.callback = default_callback, .pin = NO_PIN},
    [IRQ_2] = {.callback = default_callback, .pin = NO_PIN},
    [IRQ_3] = {.callback = default_callback, .pin = NO_PIN},
    [IRQ_4] = {.callback = default_callback, .pin = NO_PIN},
    [IRQ_9_5] = {.callback = default_callback, .pin = NO_PIN},
    [IRQ_15_10] = {.callback = default_callback, .pin = NO_PIN},
};

void EXTI15_10_IRQHandler() {
  HAL_GPIO_EXTI_IRQHandler(interrupts[IRQ_15_10].pin);
}

void EXTI9_5_IRQHandler() {
  HAL_GPIO_EXTI_IRQHandler(interrupts[IRQ_9_5].pin);
}

void EXTI4_IRQHandler() {
  HAL_GPIO_EXTI_IRQHandler(interrupts[IRQ_4].pin);
}

void EXTI3_IRQHandler() {
  HAL_GPIO_EXTI_IRQHandler(interrupts[IRQ_3].pin);
}

void EXTI2_IRQHandler() {
  HAL_GPIO_EXTI_IRQHandler(interrupts[IRQ_2].pin);
}

void EXTI1_IRQHandler() {
  HAL_GPIO_EXTI_IRQHandler(interrupts[IRQ_1].pin);
}

void EXTI0_IRQHandler() {
  HAL_GPIO_EXTI_IRQHandler(interrupts[IRQ_0].pin);
}

static pin_irq categorize_pin(uint16_t pin) {
  if (pin == 0) {
    return INVALID;
  }
  int num = __builtin_ctz(pin);
  if (pin >> num != 1) {
    // More than 1 pin specified
    return INVALID;
  }
  if (num >= 10) {
    return IRQ_15_10;
  }
  if (num >= 5) {
    return IRQ_9_5;
  }
  if (num == 4) {
    return IRQ_4;
  }
  if (num == 3) {
    return IRQ_3;
  }
  if (num == 2) {
    return IRQ_2;
  }
  if (num == 1) {
    return IRQ_1;
  }
  return IRQ_0;
}

static IRQn_Type pin_category_to_IRQn(pin_irq pin_cat) {
  switch (pin_cat) {
  case IRQ_0:
    return EXTI0_IRQn;
  case IRQ_1:
    return EXTI1_IRQn;
  case IRQ_2:
    return EXTI2_IRQn;
  case IRQ_3:
    return EXTI3_IRQn;
  case IRQ_4:
    return EXTI4_IRQn;
  case IRQ_9_5:
    return EXTI9_5_IRQn;
  case IRQ_15_10:
    return EXTI15_10_IRQn;
  case INVALID:
    // Seems to be the `invalid` interrupt
    return WWDG_IRQn;
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  pin_irq index = categorize_pin(GPIO_Pin);
  if (index == INVALID) {
    return;
  }
  if (interrupts[index].pin == GPIO_Pin) {
    interrupts[index].callback();
  }
}

GPIO_EXTI_Err_t gpio_exti_register_callback(
    uint16_t pin, void (*callback)(void), bool overwrite
) {
  pin_irq index = categorize_pin(pin);
  if (index == INVALID) {
    return GPIO_EXTI_ERR_INVALID_PIN;
  }

  IRQn_Type irqn = pin_category_to_IRQn(index);
  HAL_NVIC_EnableIRQ(irqn);

  if (!overwrite && interrupts[index].pin != NO_PIN &&
      interrupts[index].callback != default_callback &&
      interrupts[index].callback != NULL) {
    return GPIO_EXTI_ERR_REGISTERED_CALLBACK;
  }
  interrupts[index] = (InterruptInfo){.pin = pin, .callback = callback};
  return GPIO_EXTI_ERR_OK;
}

GPIO_EXTI_Callback gpio_exti_get_callback(uint16_t pin) {
  pin_irq index = categorize_pin(pin);
  if (index == INVALID) {
    return NULL;
  }
  if (interrupts[index].pin != pin) {
    return NULL;
  }
  GPIO_EXTI_Callback ret = interrupts[index].callback;
  if (ret == default_callback) {
    return NULL;
  }

  return ret;
}
