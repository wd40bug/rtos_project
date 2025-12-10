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

// Communicate with the ESP32
task_err uart_task(task_data* task) {
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
    // Get reading from i2c
    err = message_queue_read(i2c_queue, &reading, sizeof(reading));
    if (err != MESSAGE_QUEUE_OK) {
      printf("Error reading from i2c_queue: %u\n", err);
      return GEN_ERR;
    }
    // Write to ESP
    int written = snprintf(
        cbuffer,
        CBUFFER_LENGTH,
        "lamp, %d, %d, %d, %d, 0\ncharger, %d, %d, %d, %d, 0\n",
        (GPIOA->ODR & (1 << 5)) >> 5,
        reading.v1,
        reading.i1,
        reading.p1,
        (GPIOA->ODR & (1 << 6)) >> 6,
        reading.v2,
        reading.v2,
        reading.p2
    );
    if (written >= CBUFFER_LENGTH) {
      printf("I guess %u wasn't enough\n", CBUFFER_LENGTH);
      return GEN_ERR;
    }
    // printf("Sending to ESP:\n%.*s", CBUFFER_LENGTH, cbuffer);
    write_uart(cbuffer, written);
    // Read 2 messages from ESP
    for (int i = 0; i < 2; i++) {
      int read = read_uart_until(cbuffer, CBUFFER_LENGTH, '\n');
      if (cbuffer[read - 1] != '\n') {
        printf("Couldn't find '\\n', found: '%*s' \n", read, cbuffer);
        ;
        return GEN_ERR;
      }
      if (read - 1 == ACK_SIZE && strncmp(cbuffer, ACK_NAME, ACK_SIZE) == 0) {
        // Nothing, ACK message
      } else if (read == RELAY_MSG_SIZE + 1) {
        // Maybe a Relay message?
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
        printf("Received from ESP: %.*s\n", read - 1, cbuffer);
        RelayCommand relay_msg = {.relay = relay};
        // Send to relay task
        message_queue_write(relay_queue, &relay_msg, sizeof(relay_msg));
      } else {
        malformed_message(cbuffer, read);
        continue;
      }
    }
  }
}

// task_err fake_i2c(task_data* task) {
//   printf("Fake i2c task\n");
//   MESSAGE_QUEUE_HANDLE Q;
//   message_q_error err = message_queue_create(uart_task_handle, &Q);
//   if (err != MESSAGE_QUEUE_OK) {
//     return GEN_ERR;
//   }
//   while (1) {
//     SensorReading reading = {.v1 = 69, .i1 = 100, .v2 = 420, .i2 = 200};
//     printf("Sending fake data\n");
//     message_queue_write(Q, &reading, sizeof(reading));
//     delay_ms(1000);
//   }
// }

// Standard config forscontinuous measurements
const uint16_t I2C_CONFIG = 0x199F; // 0011 1001 1001 1111
const uint16_t I2C_CALIB1 = 136;    // 0111 0100 0101 1000
const uint16_t I2C_CALIB2 = 38;     // 0111 0100 0101 1000
const uint8_t SL1 = 0x40;
const uint8_t SL2 = 0x41;

task_err i2c_task(task_data* task) {
  // Read from INA219
  printf("Beginning i2c task\n");
  MESSAGE_QUEUE_HANDLE Q;
  message_q_error err = message_queue_create(uart_task_handle, &Q);
  if (err != MESSAGE_QUEUE_OK) {
    return GEN_ERR;
  }

  i2c_write_reg(SL1, CONFIGURATION, I2C_CONFIG);
  i2c_write_reg(SL2, CONFIGURATION, I2C_CONFIG);
  printf("Sent configs to i2cs\n");
  i2c_write_reg(SL1, CALIBRATION, I2C_CALIB1);
  i2c_write_reg(SL2, CALIBRATION, I2C_CALIB2);
  printf("Sent calibration to i2cs\n");
  while (1) {
    uint16_t voltage1 = 0xFFFF;
    uint16_t current1 = 0xFFFF;
    uint16_t power1 = 0xFFFF;
    i2c_read_reg(SL1, SHUNT_VOLTAGE, &voltage1);
    i2c_read_reg(SL1, CURRENT, &current1);
    i2c_read_reg(SL1, POWER, &power1);
    uint16_t voltage2 = 0xFFFF;
    uint16_t current2 = 0xFFFF;
    uint16_t power2 = 0xFFFF;
    i2c_read_reg(SL2, SHUNT_VOLTAGE, &voltage2);
    i2c_read_reg(SL2, CURRENT, &current2);
    i2c_read_reg(SL2, POWER, &power2);

    // From the datasheet
    float voltage1_mV = voltage1 * 0.1f;
    float current1_mA = current1 * 0.03f;
    float power1_mW = power1 * 0.003f * 20.0f;

    float voltage2_mV = voltage2 * 0.1f;
    float current2_mA = current2 * 0.106f;
    float power2_mW = power2 * 0.0106f * 20.0f;

    SensorReading reading = {
        .i1 = current1_mA,
        .v1 = voltage1_mV,
        .p1 = power1_mW,
        .i2 = current2_mA,
        .v2 = voltage2_mV,
        .p2 = power2_mW
    };
    message_queue_write(Q, &reading, sizeof(reading));
    delay_ms(100);
  }
}

// Set and reset relays
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
