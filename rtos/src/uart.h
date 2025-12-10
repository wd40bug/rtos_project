#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialize UART system
 */
void uart_init();
/**
 * @brief Write to UART
 *
 * @param data 
 * @param num 
 */
void write_uart(char* data, size_t num);
/**
 * @brief Read from uart
 *
 * @param buffer 
 * @param size 
 */
void read_uart(char* buffer, size_t size);
/**
 * @brief Read from UART until to_find
 *
 * @param buffer 
 * @param max_size 
 * @param to_find 
 * @return 
 */
size_t read_uart_until(char* buffer, size_t max_size, char to_find);

#endif /* ifndef UART_H */
