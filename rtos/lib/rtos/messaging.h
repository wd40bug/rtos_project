#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include "rtos.h"
#include "scheduling.h"

#define MESSAGE_QUEUE_SIZE 512
#define MAX_QUEUES_PER_TASK MAX_TASKS

void messaging_init();

message_q_error messaging_queue_create(
    TASK_HANDLE handle, MESSAGE_QUEUE_HANDLE* q_handle
);
message_q_error message_queue_write(
    MESSAGE_QUEUE_HANDLE q_handle, void*const data, size_t size
);
message_q_error message_queue_read(
    MESSAGE_QUEUE_HANDLE q_handle, void* data, size_t buffer_size
);

message_q_error messaging_close_queues( 
    TASK_HANDLE handle
);

message_q_error message_queue_data_available(MESSAGE_QUEUE_HANDLE q_handle, size_t* data);

#endif /* ifndef MESSAGE_QUEUE_H */
