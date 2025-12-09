// src/stm_link.h
#pragma once
#include <Arduino.h>

// Initializes communication with the STM32 over UART
// serialPort: reference to a HardwareSerial instance (e.g., Serial2)
// baud: baud rate for UART communication
// Must be called once inside setup() before any STM32 interaction
void stmInit(HardwareSerial &serialPort, uint32_t baud);

// Polls the STM32 for new UART data and processes any received lines
// Should be called regularly in loop() to keep communication responsive
// Reads incoming data, updates load states, and sends ACK or queued commands back
void stmPoll();

// Adds a relay toggle command to the outgoing UART queue
// index: which relay to control (0=lamp, 1=charger, 2=fan)
// on: requested logical state (used for ESP bookkeeping; STM just toggles)
// Returns true if command was added successfully, false if queue is full or index is invalid
bool stmQueueRelayCommand(int index, bool on);
