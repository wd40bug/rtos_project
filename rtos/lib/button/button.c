#include "button.h"
#include "common.h"
#include "gpio_exti.h"

void button_init() {
  BUTTON_GPIO_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = BUTTON_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);
}

bool button_is_pressed() {
  return HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_PIN) == LOW;
}

static void (*pressed_callback)(void) = default_callback;
static void (*released_callback)(void) = default_callback;
static void (*changed_callback)(void) = default_callback;

static void button_callback(void) {
  // Note this assumes pulses are long enough to last
  if (button_is_pressed()) {
    pressed_callback();
  } else {
    released_callback();
  }
  changed_callback();
}

static ButtonErr button_register_callback(
    void (*callback)(void), uint32_t trigger, ButtonOverride override_flags
) {
  if (gpio_exti_get_callback(BUTTON_PIN) != button_callback) {
    // Override (potentially existing) callback
    GPIO_EXTI_Err_t gpio_exti_err = gpio_exti_register_callback(
        BUTTON_PIN,
        button_callback,
        override_flags & BUTTON_ERR_OVERRIDE_GPIO_EXTI_CALLBACK
    );
    if (gpio_exti_err == GPIO_EXTI_ERR_REGISTERED_CALLBACK) {
      return BUTTON_ERR_OVERRIDE_GPIO_EXTI_CALLBACK;
    }
    if (gpio_exti_err != GPIO_EXTI_ERR_OK) {
      return BUTTON_ERR_GPIO_EXTI_ERR;
    }
  }
  return BUTTON_ERR_OK;
}

ButtonErr button_on_pressed(void (*callback)(void), ButtonOverride override) {
  if (pressed_callback != default_callback &&
      !(override & BUTTON_OVERRIDE_BUTTON_CALLBACK)) {
    return BUTTON_ERR_OVERRIDE_BUTTON_CALLBACK;
  }
  ButtonErr err =
      button_register_callback(callback, EXTI_TRIGGER_FALLING, override);
  if (err != BUTTON_ERR_OK) {
    return err;
  };
  pressed_callback = callback;
  return BUTTON_ERR_OK;
}

ButtonErr button_on_released(void (*callback)(void), ButtonOverride override) {
  if (released_callback != default_callback &&
      !(override & BUTTON_OVERRIDE_BUTTON_CALLBACK)) {
    return BUTTON_ERR_OVERRIDE_BUTTON_CALLBACK;
  }
  ButtonErr err =
      button_register_callback(callback, EXTI_TRIGGER_FALLING, override);
  if (err != BUTTON_ERR_OK) {
    return err;
  };
  released_callback = callback;
  return BUTTON_ERR_OK;
}

ButtonErr button_on_change(void (*callback)(void), ButtonOverride override) {
  if (changed_callback != default_callback &&
      !(override & BUTTON_OVERRIDE_BUTTON_CALLBACK)) {
    return BUTTON_ERR_OVERRIDE_BUTTON_CALLBACK;
  }
  ButtonErr err =
      button_register_callback(callback, EXTI_TRIGGER_FALLING, override);
  if (err != BUTTON_ERR_OK) {
    return err;
  };
  changed_callback = callback;
  return BUTTON_ERR_OK;
}
