#include <stdint.h>
#include "rtos.h"

#define MAX_DELAYS MAX_TASKS

void timing_init();
void timing_tick();
uint64_t timing_get_ticks();

timer_err timing_delay_ms(uint32_t ms);
