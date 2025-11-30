// src/stm_link.h
#pragma once
#include <Arduino.h>

// Call once in setup()
void stmInit(HardwareSerial &serialPort, uint32_t baud);

// Call frequently in loop()
// This will read from STM32 (via UART) and update g_loads in loads.cpp
void stmPoll();
