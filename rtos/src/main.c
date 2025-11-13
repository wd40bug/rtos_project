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

int main(void) {
  rtos_init();
  scheduling_add_task(ping, 0);
  scheduling_add_task(pong, 0);
  printf("Everything Initialized!\n");
  rtos_run();
  while (1) {
  }
}
