#ifndef SCHEDULING_H
#define SCHEDULING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  OK = 0,
  GEN_ERR = 1,
} task_err;

typedef enum {
  SCHED_ERR_OK = 0,
  SCHED_ERR_TOO_MANY_TASKS,
  SCHED_ERR_UNEXPECTED_RETURN,
} sched_err;

typedef struct {
  size_t task_handle;
} task_data;

typedef task_err (*TASK)(task_data*);

sched_err scheduling_init();
sched_err scheduling_add_task(TASK task);
sched_err scheduling_run();

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

#endif /* ifndef SCHEDULING_H */
