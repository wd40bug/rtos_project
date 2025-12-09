// src/loads.h
#pragma once
#include <Arduino.h>

// Enumeration for load IDs used to identify each controlled device
enum LoadId {
  LOAD_LAMP = 0,     // Index 0 - Desk Lamp relay
  LOAD_CHARGER = 1,  // Index 1 - Phone Charger relay
  LOAD_FAN = 2,      // Index 2 - Fan relay
  LOAD_COUNT          // Total number of loads
};

// Structure describing the current state of one electrical load
struct LoadState {
  const char *id;     // Short string identifier ("lamp", "charger", "fan")
  const char *name;   // Human-readable display name for UI
  bool on;            // Logical on/off state tracked by ESP32
  float voltage_V;    // Measured voltage in volts
  float current_A;    // Measured current in amps
  float power_W;      // Instantaneous power in watts
  float energy_Wh;    // Accumulated energy usage in watt-hours
};

// Global array of load states (allocated in loads.cpp)
extern LoadState g_loads[LOAD_COUNT];

// Initializes the g_loads array with default IDs, names, and zeros for measurements
void initLoads();

// Finds a load pointer by its string identifier ("lamp", "charger", or "fan")
LoadState *findLoadById(const String &id);

// Generates fake measurement data when STM32 is disconnected (for UI demo/testing)
void updateFakeLoads();

// Computes total power draw across all monitored loads (lamp + charger)
float getTotalPowerW();
