#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

void uart_init();
void write_uart(char* data, size_t num);
void read_uart(char* buffer, size_t size);
size_t read_uart_until(char* buffer, size_t max_size, char to_find);

#endif /* ifndef UART_H */
