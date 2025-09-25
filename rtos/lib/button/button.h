#ifndef BUTTON_H
#define BUTTON_H

#include "common.h"
#include <stdbool.h>

#define BUTTON_PIN GPIO_PIN_13
#define BUTTON_GPIO_PORT GPIOC

#define BUTTON_IRQn EXTI15_10_IRQn
#define BUTTON_EXTI_LINE EXTI_LINE_13
#define BUTTON_EXTI_GPIO EXTI_GPIOC

#define BUTTON_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

typedef enum {
  BUTTON_ERR_OK,
  BUTTON_ERR_OVERRIDE_BUTTON_CALLBACK,
  BUTTON_ERR_OVERRIDE_GPIO_EXTI_CALLBACK,
  BUTTON_ERR_GPIO_EXTI_ERR,
} ButtonErr;

/**
 * @brief Initialize on-board button
 */
void button_init();

/**
 * @brief Returns if button is currently pressed
 *
 * @return
 */
bool button_is_pressed();

typedef uint8_t ButtonOverride;

enum {
  // Don't override anything
  BUTTON_OVERRIDE_NONE = 0,
  // Override interrupt for the EXTI line saved in the gpio_exti module
  BUTTON_OVERRIDE_GPIO_EXTI_CALLBACK = 1,
  // Override interrupt this module has saved
  BUTTON_OVERRIDE_BUTTON_CALLBACK = 1 << 1,
  // Override both
  BUTTON_OVERRIDE_ALL =
      BUTTON_OVERRIDE_GPIO_EXTI_CALLBACK | BUTTON_OVERRIDE_BUTTON_CALLBACK,
};

/**
 * @brief Register callback on posedge of on-board button
 * Must be called after `button_Init`
 *
 * @param callback
 * @param override flags of whether to override any existing interrupts
 * @retval BUTTON_ERR_OK success
 * @retval BUTTON_ERR_OVERRIDE_BUTTON_CALLBACK if `OVERRIDE_BUTTON_CALLBACK` isn't set and interrupt already registered for press_button
 * @retval BUTTON_ERR_OVERRIDE_GPIO_EXTI_CALLBACK if `OVERRIDE_GPIO_EXTI_CALLBACK` isn't set and interrupt already registered for button EXTI line
 * @retval BUTTON_ERR_GPIO_EXTI_ERR if the gpio_exti module had an error
 */
ButtonErr button_on_pressed(void (*callback)(void), ButtonOverride override);
/**
 * @brief Register callback on negedge of on-board button
 * Must be called after `button_Init`
 *
 * @param callback
 */
ButtonErr button_on_released(void (*callback)(void), ButtonOverride override);
/**
 * @brief Register callback on change of on-baord button
 *
 * @param callback
 */
ButtonErr button_on_change(void (*callback)(void), ButtonOverride override);

#endif // BUTTON_H
