#include "svc.h"
#include <printf.h>
#include <stdbool.h>
#include <inttypes.h>

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

task_err long_calculation(task_data* task) {
  printf("Starting calculation\n");
  volatile uint32_t accumulator = 0;
  uint32_t operations = 0;
  while (1) {
    accumulator += operations;
    operations++;

    if (operations == 10000000) {
      printf("Performed ten million additions\nTime is: %lu\n", (uint32_t)ms_since_start());
      operations = 0;
    }
  }
}

int main(void) {
  rtos_init();
  scheduling_add_task(ping, 0, NULL);
  scheduling_add_task(pong, 0, NULL);
  scheduling_add_task(long_calculation, 1, NULL);
  printf("Everything Initialized!\n");
  rtos_run();
  while (1) {
  }
}
