#include "rtos.h"

void gain_priviledge();
void relinquish_priviledge();
timer_err delay_ms(uint32_t ms);

message_q_error message_queue_create(
    TASK_HANDLE handle, MESSAGE_QUEUE_HANDLE* q_handle
);
