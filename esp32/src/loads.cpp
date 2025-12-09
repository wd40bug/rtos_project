// src/loads.cpp
#include "loads.h"
#include <string.h>  // for strcmp

// Global array holding the state of all loads
LoadState g_loads[LOAD_COUNT];

// Initialize the g_loads array with default values for each load
void initLoads() {
  g_loads[LOAD_LAMP] = {
    "lamp",           // ID used for identification
    "Desk Lamp",      // Human-readable name
    true,             // Initial ON state
    5.0f,             // Voltage in volts
    0.25f,            // Current in amps
    30.0f,            // Power in watts
    12.5f             // Energy in watt-hours
  };

  g_loads[LOAD_CHARGER] = {
    "charger",        // ID
    "Phone Charger",  // Name
    true,             // Initial ON state
    5.0f,             // Voltage in volts
    1.2f,             // Current in amps
    6.0f,             // Power in watts
    3.4f              // Energy in watt-hours
  };

  // Fan is actuator only, not monitored for voltage/current/power
  g_loads[LOAD_FAN] = {
    "fan",            // ID
    "Desk Fan",       // Name
    false,            // Initially OFF
    0.0f,             // No voltage measurement
    0.0f,             // No current measurement
    0.0f,             // No power measurement
    0.0f              // No energy tracking
  };
}

// Searches for a load by its string identifier and returns a pointer to it
LoadState *findLoadById(const String &id) {
  for (int i = 0; i < LOAD_COUNT; ++i) {
    if (id == g_loads[i].id) {
      return &g_loads[i];  // Return pointer to matching load
    }
  }
  return nullptr;          // Return null if not found
}

// Generates fake data to simulate load readings when STM32 is not connected
void updateFakeLoads() {
  // Only create fake readings for monitored loads (lamp + charger)
  for (int i = 0; i < LOAD_COUNT; ++i) {
    LoadState &l = g_loads[i];

    // Skip fan since it is only controlled, not measured
    if (strcmp(l.id, "fan") == 0) {
      continue;
    }

    if (!l.on) {
      // When the load is OFF, set all readings to zero
      l.current_A = 0.0f;
      l.power_W = 0.0f;
      continue;
    }

    // If ON but power reading is nearly zero, reset to default nominal value
    if (l.power_W <= 0.01f) {
      const String id = l.id;
      if (id == "lamp") {
        l.current_A = 0.25f;   // Same as initial setup for lamp
      } else if (id == "charger") {
        l.current_A = 1.2f;    // Same as initial setup for charger
      } else {
        l.current_A = 0.5f;    // Fallback value for unexpected case
      }

      l.power_W = l.voltage_V * l.current_A;  // Recalculate power
    }

    // Apply small random noise to simulate measurement fluctuations
    float noise = (random(-5, 6)) / 100.0f;   // Range -0.05 to +0.05 (±5%)
    float newCurrent = l.current_A * (1.0f + noise);
    if (newCurrent < 0) newCurrent = 0;       // Clamp to non-negative
    l.current_A = newCurrent;
    l.power_W = l.voltage_V * l.current_A;    // Update power based on new current

    // Energy accumulation disabled (uncomment for integration testing)
    // l.energy_Wh += l.power_W / 3600.0f;
  }
}

// Computes total power across all monitored loads (lamp + charger only)
float getTotalPowerW() {
  float sum = 0.0f;
  for (int i = 0; i < LOAD_COUNT; ++i) {
    // Skip the fan (not monitored)
    if (strcmp(g_loads[i].id, "fan") == 0) {
      continue;
    }
    sum += g_loads[i].power_W;  // Add each monitored load's power
  }
  return sum / 1000;            // Convert from milliwatts to watts if stored in mW
}
