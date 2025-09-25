#ifndef GPIO_EXTI_H
#define GPIO_EXTI_H

#include "common.h"
#include <stdbool.h>

// Hey what is the most god-awful function pointer syntax possible?
// "Hold my beer" - Dennis Ritchie
typedef void (*GPIO_EXTI_Callback)(void);

typedef enum {
  // No Error
  GPIO_EXTI_ERR_OK = 0,
  // Already have a registered callback for this line
  GPIO_EXTI_ERR_REGISTERED_CALLBACK,
  // Invalid pin or too many pins
  GPIO_EXTI_ERR_INVALID_PIN,
} GPIO_EXTI_Err_t;

/**
 * @brief Register a GPIO callback
 *
 * @param pin
 * @param callback
 * @param overwrite whether to overwrite existing callbacks on this EXTI line
 * @retval GPIO_EXTI_ERR_OK success
 * @retval GPIO_EXTI_INVALID_PIN invalid `pin`
 * @retval GPIO_EXTI_ERR_REGISTERED_CALLBACK would have to register callback without `overwrite` being true
 */
GPIO_EXTI_Err_t gpio_exti_register_callback(
    uint16_t pin, void (*callback)(void), bool overwrite
);

/**
 * @brief Get the callback for the given pin
 *
 * @param pin 
 * @return 
 */
GPIO_EXTI_Callback gpio_exti_get_callback(uint16_t pin);

/**
 * @brief Not meant to be called directly, doesn't do anything
 */
void default_callback(void);

#endif /* ifndef GPIO_EXTI_H */
