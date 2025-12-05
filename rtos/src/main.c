#include "rtos.h"
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

typedef enum {
  CONFIGURATION =0,
  SHUNT_VOLTAGE = 1,
  BUS_VOLTAGE = 2,
  POWER = 3,
  CURRENT = 4,
  CALIBRATION = 5
} i2c_register;

I2C_Error get_slave_addr(I2C_TypeDef* i2c, uint8_t* addr) {
  switch ((uintptr_t)i2c) {
  case (uintptr_t)I2C1:
    *addr = 0x41;
    return I2C_OK;
  case (uintptr_t)I2C2:
    *addr = 0x41;
    return I2C_OK;
  default:
    return INVALID_I2C;
  }
}

I2C_Error write_i2c_message(I2C_TypeDef* i2c, i2c_register reg, uint16_t data) {
  uint8_t slave_addr;
  I2C_Error err = get_slave_addr(i2c, &slave_addr);
  if (err != I2C_OK) {
    return err;
  }
  union {
    uint8_t bytes[2];
    uint16_t val;
  } send;
  send.val = data;
  uint8_t to_send[3] = {reg, send.bytes[1], send.bytes[0]};
  return i2c_write(i2c, slave_addr, to_send, 3);
}

I2C_Error read_i2c_message(I2C_TypeDef* i2c, uint16_t* dout) {
  union {
    uint8_t bytes[2];
    uint16_t value;
  } read;
  uint8_t slave_addr;
  I2C_Error err = get_slave_addr(i2c, &slave_addr);
  if (err != I2C_OK){
    return err;
  }
  err = i2c_read(i2c, slave_addr, read.bytes, 2);
  if (err != I2C_OK) {
    return err;
  }
  *dout = read.value;
  return I2C_OK;
}
const uint16_t I2C_CONFIG = 0x399F; // 0011 1001 1001 1111
const uint16_t I2C_CALIB = 13400; // 0111 0100 0101 1000

task_err read1(task_data* task) {
  // Standard config for continuous measurements
  printf("Beginning i2c1 task\n");
  write_i2c_message(I2C1, CONFIGURATION, I2C_CONFIG);
  printf("Sent configs\n");
  write_i2c_message(I2C1, CALIBRATION, I2C_CALIB);
  printf("Sent calibration\n");
  uint16_t voltage = 0xFFFF;
  read_i2c_message(I2C1, &voltage);
  printf("READ FROM i2c: %u\n", voltage);
  while (1) {
  }
}

task_err read2(task_data* task) {
  printf("Beginning i2c2 task\n");
  while (1);
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
  if (s_err != SCHED_ERR_OK) {
    return GEN_ERR;
  }
  MESSAGE_QUEUE_HANDLE print_queue;
  message_q_error q_err = message_queue_create(print_task, &print_queue);
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
  scheduling_add_task(read1, 0, NULL);
  printf("\nEverything Initialized!\n");
  rtos_run();
  while (1) {
  }
}
