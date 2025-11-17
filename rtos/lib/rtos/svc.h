#include "rtos.h"

/**
 * @brief UNUSED. gain priviledge (ARM feature)
 */
void gain_priviledge(); 
/**
 * @brief UNUSED. Relinquish priviledge (ARM feature)
 */
void relinquish_priviledge();
timer_err delay_ms(uint32_t ms); // Documented in rtos.h

message_q_error message_queue_create(
    TASK_HANDLE handle, MESSAGE_QUEUE_HANDLE* q_handle
); // Documented in rtos.h
