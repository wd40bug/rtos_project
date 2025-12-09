#ifndef SERIAL_H
#define SERIAL_H
#include <stdint.h>

/**
 * @brief Initialize the serial system
 *
 * @param baud 
 */
void init_serial(uint32_t baud);
/**
 * @brief SHOULD ONLY BE CALLED FROM SVC. Sends the character to be printed
 *
 * @param character 
 */
void serial_send_char(char character);

#endif /* SERIAL_H */
