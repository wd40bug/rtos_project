#include <stdint.h>
#include "rtos.h"

#define MAX_DELAYS MAX_TASKS

/**
 * @brief Initialize the timing mechanism
 */
void timing_init();
/**
 * @brief Actions performed every 1000ms. Only to be called from SysTick_Handler
 */
void timing_tick();
/**
 * @brief Get the number of ticks since timing_init
 *
 * @return 
 */
uint64_t timing_get_ticks();

/**
 * @brief Delay the current task for the number of ms. ONLY CALL FROM SVC
 *
 * @param ms 
 * @return 
 */
timer_err timing_delay_ms(uint32_t ms);
