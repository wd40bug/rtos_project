#ifndef RTOS_H
#define RTOS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//-------DEFINITIONS----------
typedef uint32_t TASK_HANDLE;
typedef size_t MESSAGE_QUEUE_HANDLE;

//--------RTOS----------------
/**
 * @brief Initialize the RTOS, should be done first thing
 */
void rtos_init();
/**
 * @brief Run the RTOS, begins task switching. Should be called after adding some tasks
 */
void rtos_run();

//--------SCHEDULING----------
typedef enum {
  OK = 0,
  GEN_ERR = 1,
} task_err;

typedef struct {
  size_t task_handle;
  uint32_t priority;
} task_data;

typedef task_err (*TASK)(task_data*);

typedef enum {
  SCHED_ERR_OK = 0,
  SCHED_ERR_TOO_MANY_TASKS,
  SCHED_ERR_UNEXPECTED_RETURN,
  SCHED_ERR_NOT_RUNNING,
  SCHED_ERR_INVALID_PRIORITY,
  SCHED_ERR_NOT_ASLEEP,
  SCHED_ERR_NOT_READY,
  SCHED_ERR_INVALID_HANDLE,
} sched_err;

/**
 * @brief Add a task to the scheduler
 *
 * @param task
 * @param priority 0 is the highest priority PRIORITIES - 1 is the lowest
 * @paramout handle
 * @return
 */
extern sched_err scheduling_add_task(
    TASK task, uint32_t priority, TASK_HANDLE* handle
);
/**
 * @brief Yield the current task without sleeping
 *
 * @return
 */
extern sched_err yield();
#define PRIORITIES 2

//--------TIMING--------------
typedef enum {
  TIMER_OK,
  TIMER_TOO_LARGE,
  TIMER_TOO_MANY_TIMERS,
  TIMER_SCHED_ERR,
} timer_err;
/**
 * @brief Sleep the current task until delay ms has passed
 *
 * @param delay
 * @return
 */
extern timer_err delay_ms(uint32_t delay); // Syscall
/**
 * @brief The number of milliseconds since rtos_init
 *
 * @return 
 */
extern uint64_t ms_since_start();

//-------MESSAGE-QUEUES-------
typedef enum {
  MESSAGE_QUEUE_OK,
  MESSAGE_QUEUE_TOO_MANY_QUEUES,
  MESSAGE_QUEUE_INVALID_HANDLE,
  MESSAGE_QUEUE_CLOSED,
  MESSAGE_QUEUE_DEADLOCK,
  MESSAGE_QUEUE_INVALID_TASK
} message_q_error;
/**
 * @brief Create a message queue between the current task and another task
 *
 * @param handle 
 * @param q_handle 
 * @return 
 */
extern message_q_error message_queue_create(
    TASK_HANDLE handle, MESSAGE_QUEUE_HANDLE* q_handle
); // Syscall
/**
 * @brief Write to the message queue (sleep if can't write enough data)
 *
 * @param q_handle 
 * @paramout data 
 * @param size 
 * @return 
 */
extern message_q_error message_queue_write(
    MESSAGE_QUEUE_HANDLE q_handle, void* const data, size_t size
);
/**
 * @brief Read from the message queue (sleep if can't read enough data)
 *
 * @param q_handle 
 * @param data 
 * @param size 
 * @return 
 */
extern message_q_error message_queue_read(
    MESSAGE_QUEUE_HANDLE q_handle, void* data, size_t size
);
/**
 * @brief Number of bytes available to read from the queue
 *
 * @param q_handle 
 * @paramout data 
 * @return 
 */
extern message_q_error message_queue_data_available(
    MESSAGE_QUEUE_HANDLE q_handle, size_t* data
);

#endif
