// src/loads.h
#pragma once
#include <Arduino.h>

enum LoadId {
  LOAD_LAMP = 0,
  LOAD_CHARGER = 1,
  LOAD_FAN = 2,
  LOAD_COUNT
};

struct LoadState {
  const char *id;     // "lamp", "charger", "fan"
  const char *name;   // pretty display name
  bool on;
  float voltage_V;
  float current_A;
  float power_W;
  float energy_Wh;
};

// Global array of loads (defined in loads.cpp)
extern LoadState g_loads[LOAD_COUNT];

// Initialize with default values
void initLoads();

// Find load by string id ("lamp"/"charger"/"fan")
LoadState *findLoadById(const String &id);

// Simple fake update to wiggle values when STM32 isn’t hooked up yet
void updateFakeLoads();

// Utility: total power across all loads (for graphs etc.)
float getTotalPowerW();
