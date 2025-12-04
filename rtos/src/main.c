#include "svc.h"
#include <inttypes.h>
#include <printf.h>
#include <stdbool.h>
#include "I2C.h"

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

task_err ina219_task(task_data* task) {
  (void)task;  // unused if your RTOS does not need it

  INA219_Device dev1;
  INA219_Device dev2;

  INA219_Status status;

  // Initialize INA219 on I2C1 @ 0x40
  status = INA219_Init(&dev1, I2C1, INA219_ADDR_40);
  if (status != INA219_OK) {
    printf("INA219 init failed for dev1 (0x40)\n");
    return GEN_ERR;
  }

  // Initialize INA219 on I2C2 @ 0x41
  status = INA219_Init(&dev2, I2C2, INA219_ADDR_41);
  if (status != INA219_OK) {
    printf("INA219 init failed for dev2 (0x41)\n");
    return GEN_ERR;
  }

  printf("INA219 sensors initialized.\n");

  while (1) {
    float v1_V, i1_mA, p1_W;
    float v2_V, i2_mA, p2_W;

    // Read sensor 1
    if (INA219_ReadBusVoltage_V(&dev1, &v1_V) != INA219_OK ||
        INA219_ReadCurrent_mA(&dev1, &i1_mA) != INA219_OK ||
        INA219_ReadPower_W(&dev1, &p1_W) != INA219_OK) {
      printf("Error reading INA219 dev1\n");
      return GEN_ERR;
    }

    // Read sensor 2
    if (INA219_ReadBusVoltage_V(&dev2, &v2_V) != INA219_OK ||
        INA219_ReadCurrent_mA(&dev2, &i2_mA) != INA219_OK ||
        INA219_ReadPower_W(&dev2, &p2_W) != INA219_OK) {
      printf("Error reading INA219 dev2\n");
      return GEN_ERR;
    }

    // Scale to integers for printing (mV, mA, mW)
    int32_t v1_mV = (int32_t)(v1_V * 1000.0f);
    int32_t p1_mW = (int32_t)(p1_W * 1000.0f);
    int32_t v2_mV = (int32_t)(v2_V * 1000.0f);
    int32_t p2_mW = (int32_t)(p2_W * 1000.0f);

    printf("=== INA219 readings ===\n");
    printf("Sensor 1 @ 0x40: V = %" PRId32 " mV, I = %" PRId32 " mA, P = %" PRId32 " mW\n",
           v1_mV, (int32_t)i1_mA, p1_mW);
    printf("Sensor 2 @ 0x41: V = %" PRId32 " mV, I = %" PRId32 " mA, P = %" PRId32 " mW\n",
           v2_mV, (int32_t)i2_mA, p2_mW);

    delay_ms(1);
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
