#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_UART USART2

#define SERIAL_CLK_ENABLE() __HAL_RCC_USART2_CLK_ENABLE()
#define SERIAL_CLK_DISABLE() __HAL_RCC_USART2_CLK_DISABLE()

#define SERIAL_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define SERIAL_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define SERIAL_RX_GPIO_CLK_DISABLE() __HAL_RCC_GPIOA_CLK_DISABLE()
#define SERIAL_RX_GPIO_CLK_DISABLE() __HAL_RCC_GPIOA_CLK_DISABLE()

#define SERIAL_FORCE_RESET() __HAL_RCC_USART2_FORCE_RESET()
#define SERIAL_RELEASE_RESET() __HAL_RCC_USART2_RELEASE_RESET()

#define SERIAL_TX_PIN GPIO_PIN_2
#define SERIAL_TX_GPIO_PORT GPIOA
#define SERIAL_TX_AF GPIO_AF7_USART2
#define SERIAL_RX_PIN GPIO_PIN_3
#define SERIAL_RX_GPIO_PORT GPIOA
#define SERIAL_RX_AF GPIO_AF7_USART2

/**
 * @brief Initalize the serial output
 */
void serial_init();

/**
 * @brief Write to serial output
 * Must have been initialized with `serial_Init`
 *
 * @param ptr string to print
 * @param len length of string
 * @return length written before error or len
 */
int serial_write(char *ptr, int len);

#endif /* ifndef SERIAL_H */
