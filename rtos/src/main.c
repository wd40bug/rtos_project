#include "i2c.h"
#include "relays.h"
#include "rtos.h"
#include "uart.h"
#include <inttypes.h>
#include <printf.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
  CONFIGURATION = 0,
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

I2C_Error read_i2c_message(I2C_TypeDef* i2c, i2c_register reg, uint16_t* dout) {
  return write_i2c_message(i2c, reg, 0x0000);
  union {
    uint8_t bytes[2];
    uint16_t value;
  } read;
  uint8_t slave_addr;
  I2C_Error err = get_slave_addr(i2c, &slave_addr);
  if (err != I2C_OK) {
    return err;
  }
  err = i2c_read(i2c, slave_addr, read.bytes, 2);
  if (err != I2C_OK) {
    return err;
  }
  *dout = read.value;
  return I2C_OK;
}

// Standard config forscontinuous measurements
const uint16_t I2C_CONFIG = 0x399F; // 0011 1001 1001 1111
const uint16_t I2C_CALIB = 13400;   // 0111 0100 0101 1000

static TASK_HANDLE i2c_task_handle;
static TASK_HANDLE uart_task_handle;
static TASK_HANDLE relay_task_handle;

typedef struct {
  int v1;
  int i1;
  int p1;
  int v2;
  int i2;
  int p2;
} SensorReading;

typedef struct {
  uint8_t relay;
} RelayCommand;

#define CBUFFER_LENGTH 256
#define ACK_NAME "ACK"
#define ACK_SIZE (sizeof(ACK_NAME) - 1)
#define NUM_RELAYS 3
#define RELAY_MSG_SIZE 3

void malformed_message(char* str, size_t size) {
  printf("Malformed message: %.*s\n", size, str);
}

task_err uart_task(task_data* task) {
  gain_priviledge();
  printf("Starting UART task\n");
  MESSAGE_QUEUE_HANDLE i2c_queue;
  message_q_error err = message_queue_create(i2c_task_handle, &i2c_queue);
  if (err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }
  MESSAGE_QUEUE_HANDLE relay_queue;
  err = message_queue_create(relay_task_handle, &relay_queue);
  if (err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }

  while (1) {
    char cbuffer[CBUFFER_LENGTH];
    SensorReading reading;
    err = message_queue_read(i2c_queue, &reading, sizeof(reading));
    if (err != MESSAGE_QUEUE_OK) {
      printf("Error reading from i2c_queue: %u\n", err);
      return GEN_ERR;
    }
    printf("Received sensor data\n");
    int written = snprintf(
        cbuffer,
        CBUFFER_LENGTH,
        "lamp, 1, %d, %d, 0, 0\ncharger, 1, %d, %d, 0, 0\n",
        reading.v1,
        reading.i1,
        reading.v2,
        reading.v1
    );
    if (written >= CBUFFER_LENGTH) {
      printf("I guess %u wasn't enough\n", CBUFFER_LENGTH);
      return GEN_ERR;
    }
    printf("Sending to ESP: %.*s", CBUFFER_LENGTH, cbuffer);
    write_uart(cbuffer, written);
    for (int i = 0; i < 2; i++) {
      int read = read_uart_until(cbuffer, CBUFFER_LENGTH, '\n');
      if (cbuffer[read - 1] != '\n') {
        printf("Couldn't find '\\n', found: '%*s' \n", read, cbuffer);
        ;
        return GEN_ERR;
      }
      printf("Received from ESP: '%.*s' : %u chars\n", read - 1, cbuffer, read);
      if (read - 1 == ACK_SIZE && strncmp(cbuffer, ACK_NAME, ACK_SIZE) == 0) {
        // Nothing
      } else if (read == RELAY_MSG_SIZE + 1) {
        uint8_t relay;
        switch (cbuffer[0]) {
        case '0':
          relay = 0;
          break;
        case '1':
          relay = 1;
          break;
        case '2':
          relay = 2;
          break;
        default:
          malformed_message(cbuffer, read);
          continue;
        }
        RelayCommand relay_msg = {.relay = relay};
        message_queue_write(relay_queue, &relay_msg, sizeof(relay_msg));
      } else {
        malformed_message(cbuffer, read);
        continue;
      }
    }
  }
}

task_err fake_i2c(task_data* task) {
  printf("Fake i2c task\n");
  MESSAGE_QUEUE_HANDLE Q;
  message_q_error err = message_queue_create(uart_task_handle, &Q);
  if (err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }
  while (1) {
    SensorReading reading = {.v1 = 69, .i1 = 100, .v2 = 420, .i2 = 200};
    printf("Sending fake data\n");
    message_queue_write(Q, &reading, sizeof(reading));
    delay_ms(1000);
  }
}

task_err i2c_task(task_data* task) {
  printf("Beginning i2c task\n");
  MESSAGE_QUEUE_HANDLE Q;
  message_q_error err = message_queue_create(uart_task_handle, &Q);
  if (err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }

  write_i2c_message(I2C1, CONFIGURATION, I2C_CONFIG);
  write_i2c_message(I2C2, CONFIGURATION, I2C_CONFIG);
  printf("Sent configs to i2cs\n");
  write_i2c_message(I2C1, CALIBRATION, I2C_CALIB);
  write_i2c_message(I2C2, CALIBRATION, I2C_CALIB);
  printf("Sent calibration to i2cs\n");
  while (1) {
    uint16_t voltage1;
    uint16_t current1;
    uint16_t power1;
    read_i2c_message(I2C1, BUS_VOLTAGE, &voltage1);
    read_i2c_message(I2C1, CURRENT, &current1);
    read_i2c_message(I2C1, POWER, &power1);
    uint16_t voltage2;
    uint16_t current2;
    uint16_t power2;
    read_i2c_message(I2C2, BUS_VOLTAGE, &voltage2);
    read_i2c_message(I2C1, CURRENT, &current2);
    read_i2c_message(I2C1, POWER, &power2);
    SensorReading reading = {
        .i1 = current1,
        .v1 = voltage1,
        .p1 = power1,
        .i2 = current2,
        .v2 = voltage2,
        .p2 = power2
    };
    message_queue_write(Q, &reading, sizeof(reading));
    delay_ms(1000);
  }
}

task_err relay_task(task_data* task) {
  printf("Beginning relay task\n");
  MESSAGE_QUEUE_HANDLE Q;
  message_q_error err = message_queue_create(uart_task_handle, &Q);
  if (err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }
  while (1) {
    RelayCommand command;
    err = message_queue_read(Q, &command, sizeof(command));
    if (err != MESSAGE_QUEUE_OK) {
      return GEN_ERR;
    }
    toggle_relay(command.relay);
  }
}

int main(void) {
  rtos_init();
  i2c_init();
  uart_init();
  relays_init();
  scheduling_add_task(i2c_task, 0, &i2c_task_handle);
  scheduling_add_task(uart_task, 0, &uart_task_handle);
  scheduling_add_task(relay_task, 0, &relay_task_handle);
  printf("\nEverything Initialized!\n");
  rtos_run();
  while (1) {
  }
}
