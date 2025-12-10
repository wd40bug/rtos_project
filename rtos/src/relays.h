#ifndef RELAYS_H
#define RELAYS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Init relay system
 */
void relays_init();
/**
 * @brief Toggle a relay
 *
 * @param relay 
 */
void toggle_relay(uint8_t relay);

#endif /* ifndef RELAYS_H */
