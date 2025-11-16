#ifndef RTOS_H
#define RTOS_H

#include <stddef.h>
#include <stdint.h>

// TODO:
// Timing stuff
// GPIO
// UART
// SCHEDULING
// Permissions
// Stack limit
// LED

//-------DEFINITIONS----------
typedef struct __attribute__((packed)) {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t* return_address;
  uint32_t xpsr;
} ShortStackFrame;

typedef struct __attribute__ ((packed)) {
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t exc_return;
  ShortStackFrame short_frame;
} FullStackFrame;

typedef uint32_t TASK_HANDLE;
typedef uint32_t MESSAGE_QUEUE_HANDLE;

//--------RTOS----------------
void rtos_init();
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

extern sched_err scheduling_add_task(TASK task, uint32_t priority, TASK_HANDLE* handle);
extern sched_err yield();
#define PRIORITIES 2

//--------TIMING--------------
typedef enum {
  TIMER_OK,
  TIMER_TOO_LARGE,
  TIMER_TOO_MANY_TIMERS,
  TIMER_SCHED_ERR,
} timer_err;
extern timer_err delay_ms(uint32_t ms); // Syscall
extern uint64_t ms_since_start();

//--------LED-----------------
extern void led_on();
extern void led_off();

//--------GPIO----------------
extern void wait_gpio_event(); // Syscall

//--------UART----------------
extern void wait_uart();

//-------MESSAGE-QUEUES-------
typedef enum {
  MESSAGE_QUEUE_OK,
  MESSAGE_QUEUE_TOO_MANY_QUEUES,
  MESSAGE_QUEUE_EXISTS,
} message_queue_error;
extern uint32_t create_message_queue(TASK_HANDLE handle);

//--------SYSCALLS------------
extern void gain_priviledge();
extern void relinquish_priviledge();

#endif
