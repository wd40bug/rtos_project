#ifndef SCHEDULING_H
#define SCHEDULING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "rtos.h"

#define MAX_TASKS 4

sched_err scheduling_init();
sched_err scheduling_add_task(TASK task, uint32_t priority);
sched_err scheduling_run();
sched_err yield();
sched_err sleep_task(uint32_t handle);
sched_err wake_task(uint32_t handle);

void scheduling_tick();

task_data get_current_task();

#endif /* ifndef SCHEDULING_H */
