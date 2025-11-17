#include "svc.h"
#include <inttypes.h>
#include <printf.h>
#include <stdbool.h>

task_err ping(task_data* task) {
  while (1) {
    printf("ping ticks: %" PRIu32 "\n", (uint32_t)ms_since_start());
    delay_ms(1000);
  }
}

task_err pong(task_data* task) {
  while (1) {
    printf("pong ticks: %" PRIu32 "\n", (uint32_t)ms_since_start());
    delay_ms(2000);
  }
}

static TASK_HANDLE long_calculation_handle;

task_err print_time_from_long_calculation(task_data* task) {
  MESSAGE_QUEUE_HANDLE print_queue;
  message_q_error q_err =
      message_queue_create(long_calculation_handle, &print_queue);
  if (q_err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }
  uint64_t initial_time;
  q_err = message_queue_read(print_queue, &initial_time, sizeof(initial_time));
  if (q_err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }
  printf("Starting calculation at %lu\n", (uint32_t)initial_time);
  while (1) {
    uint64_t time_to_print;
    q_err =
        message_queue_read(print_queue, &time_to_print, sizeof(time_to_print));
    if (q_err != MESSAGE_QUEUE_OK) {
      return GEN_ERR;
    }
    printf(
        "Performed ten million additions\nTime is: %lu\n",
        (uint32_t)time_to_print
    );
  }
}

task_err long_calculation(task_data* task) {
  TASK_HANDLE print_task;
  sched_err s_err = scheduling_add_task(
      print_time_from_long_calculation,
      task->priority,
      &print_task
  );
  if (s_err != SCHED_ERR_OK){
    return GEN_ERR;
  }
  MESSAGE_QUEUE_HANDLE print_queue;
  message_q_error q_err =
      message_queue_create(print_task, &print_queue);
  if (q_err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }
  volatile uint32_t accumulator = 0;
  uint32_t operations = 0;
  uint64_t time = ms_since_start();
  q_err = message_queue_write(print_queue, &time, sizeof(time));
  if (q_err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }
  while (1) {
    accumulator += operations;
    operations++;

    if (operations == 10000000) {
      uint64_t time = ms_since_start();
      q_err = message_queue_write(print_queue, &time, sizeof(time));
      if (q_err != MESSAGE_QUEUE_OK) {
        return GEN_ERR;
      }
      operations = 0;
    }
  }
}

int main(void) {
  rtos_init();
  scheduling_add_task(ping, 0, NULL);
  scheduling_add_task(pong, 0, NULL);
  scheduling_add_task(long_calculation, 1, &long_calculation_handle);
  // scheduling_add_task(
  //     print_time_from_long_calculation,
  //     1,
  //     &long_calculation_print_handle
  // );
  printf("\nEverything Initialized!\n");
  rtos_run();
  while (1) {
  }
}
