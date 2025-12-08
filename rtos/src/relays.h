#ifndef RELAYS_H
#define RELAYS_H

#include <stdbool.h>
#include <stdint.h>

void relays_init();
void toggle_relay(uint8_t relay);

#endif /* ifndef RELAYS_H */
