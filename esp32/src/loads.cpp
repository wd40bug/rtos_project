// src/loads.cpp
#include "loads.h"
#include <string.h>  // for strcmp

LoadState g_loads[LOAD_COUNT];

void initLoads() {
  g_loads[LOAD_LAMP] = {
    "lamp",
    "Desk Lamp",
    true,
    120.0f,
    0.25f,
    30.0f,
    12.5f
  };

  g_loads[LOAD_CHARGER] = {
    "charger",
    "Phone Charger",
    true,
    5.0f,
    1.2f,
    6.0f,
    3.4f
  };

  // Fan = actuator only; no monitored power/energy
  g_loads[LOAD_FAN] = {
    "fan",
    "Desk Fan",
    false,
    0.0f,   // voltage not monitored
    0.0f,   // current not monitored
    0.0f,   // power not monitored
    0.0f    // energy not tracked
  };
}

LoadState *findLoadById(const String &id) {
  for (int i = 0; i < LOAD_COUNT; ++i) {
    if (id == g_loads[i].id) {
      return &g_loads[i];
    }
  }
  return nullptr;
}

void updateFakeLoads() {
  // Only fake data for *monitored* loads (lamp + charger).
  for (int i = 0; i < LOAD_COUNT; ++i) {
    LoadState &l = g_loads[i];

    // Skip fan completely: it is controlled, not measured.
    if (strcmp(l.id, "fan") == 0) {
      continue;
    }

    if (!l.on) {
      // When OFF, hard zero.
      l.current_A = 0.0f;
      l.power_W = 0.0f;
      continue;
    }

    // If it's ON but has basically 0 power, assume it was just turned back on
    // and reseed with a nominal current.
    if (l.power_W <= 0.01f) {
      const String id = l.id;
      if (id == "lamp") {
        l.current_A = 0.25f;   // same as init
      } else if (id == "charger") {
        l.current_A = 1.2f;    // same as init
      } else {
        l.current_A = 0.5f;    // fallback (shouldn't really happen)
      }

      l.power_W = l.voltage_V * l.current_A;
    }

    // Now wiggle around the current value a bit
    float noise = (random(-5, 6)) / 100.0f;  // -0.05 to +0.05
    float newCurrent = l.current_A * (1.0f + noise);
    if (newCurrent < 0) newCurrent = 0;
    l.current_A = newCurrent;
    l.power_W = l.voltage_V * l.current_A;

    // Fake energy integration assuming ~1s between updates
    l.energy_Wh += l.power_W / 3600.0f;
  }
}

float getTotalPowerW() {
  float sum = 0.0f;
  for (int i = 0; i < LOAD_COUNT; ++i) {
    // Only monitored devices (lamp + charger) contribute to total power.
    if (strcmp(g_loads[i].id, "fan") == 0) {
      continue;
    }
    sum += g_loads[i].power_W;
  }
  return sum;
}
