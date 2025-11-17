#ifndef SERIAL_H
#define SERIAL_H
#include <stdint.h>

void init_serial(uint32_t baud);
void serial_send_char(char character);

#endif /* SERIAL_H */
