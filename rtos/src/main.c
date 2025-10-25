#include "common.h"
#include "svc.h"
#include "scheduling.h"
#include "serial.h"
#include <stdbool.h>

task_err ping(task_data* task) {
  while (1) {
    printf("ping\n");
  }
}

task_err pong(task_data* task) {
  while (1) {
    printf("pong\n");
  }
}

int main(void) {
  HAL_Init();
  HAL_Delay(1000); // Allow time for monitor to connect

  SVC_Init();
  serial_init();
  scheduling_init();

  scheduling_add_task(ping);
  scheduling_add_task(pong);

  printf("Everything initialized!\n");
  scheduling_run();

  while (1) {
  }
}

// Must be present for serial to work
int _write(int file, char* ptr, int len) {
  return serial_write(ptr, len);
}
