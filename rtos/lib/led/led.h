#ifndef LED_H
#define LED_H

#include "common.h"
#define LED_PIN GPIO_PIN_5
#define LED_GPIO_PORT GPIOA
#define LED_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

/**
 * @brief Initialize the on-board LED
 */
void led_init();

/**
 * @brief Turn the on-board LED on. Must have been initialized with `led_init`
 */
void led_on();

/**
 * @brief Turn the on-board LED off. Must have been initialized with `led_init`
 */
void led_off();

/**
 * @brief Toggle the on-board LED. Must have been initialized with `led_init`
 */
void led_toggle();

/**
 * @brief Get the current state of the onboard LED. Must have been initialized with `led_init`
 *
 * @return Current state of the on-board LED
 */
Signal led_state();

#endif // LED_H
