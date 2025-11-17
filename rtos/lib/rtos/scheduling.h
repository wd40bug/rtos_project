#ifndef SCHEDULING_H
#define SCHEDULING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rtos.h"

#define MAX_TASKS 4

/**
 * @brief Initialize the scheduler
 *
 * @return
 */
sched_err scheduling_init();
sched_err scheduling_add_task(
    TASK task, uint32_t priority, TASK_HANDLE* handle
); // Documented in rtos.h
/**
 * @brief Run the scheduler
 *
 * @return 
 */
sched_err scheduling_run();
sched_err yield(); // Documented in rtos.h
/**
 * @brief Sleep the given task
 *
 * @param handle 
 * @return 
 */
sched_err sleep_task(TASK_HANDLE handle);
/**
 * @brief Wake the given task
 *
 * @param handle 
 * @return 
 */
sched_err wake_task(TASK_HANDLE handle);
/**
 * @brief Check if the given handle is valid
 *
 * @param handle 
 * @return 
 */
bool valid_handle(TASK_HANDLE handle);

/**
 * @brief Perform actions every system tick. ONLY TO BE CALLED FROM SysTick_Handler 
 * */
void scheduling_tick();

/**
 * @brief Get the current task
 *
 * @return 
 */
task_data get_current_task();

#endif /* ifndef SCHEDULING_H */
