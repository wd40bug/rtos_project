// src/stm_link.cpp
#include "stm_link.h"
#include "loads.h"

static HardwareSerial *stmSerial = nullptr;

// --- helper: split a String on ',' into up to maxParts pieces ---
static int splitCommaSeparated(const String &line, String *outParts, int maxParts) {
  int count = 0;
  int start = 0;
  int len = line.length();

  for (int i = 0; i <= len && count < maxParts; ++i) {
    if (i == len || line[i] == ',') {
      outParts[count++] = line.substring(start, i);
      start = i + 1;
    }
  }
  return count;
}

// --- handle one complete line from STM32 ---
// Expected format (no fan):
//   lamp,1,120.0,0.25,30.0,12.5
//   charger,1,5.0,1.2,6.0,3.4
//
// Fields:
//   0: id ("lamp" | "charger")
//   1: on (0 or 1)
//   2: voltage_V
//   3: current_A
//   4: power_W
//   5: energy_Wh
static void handleStmLine(const String &line) {
  String parts[6];
  int n = splitCommaSeparated(line, parts, 6);
  if (n < 2) {
    // Not even id + on; nothing we can do.
    Serial.print("STM parse: too few fields: ");
    Serial.println(line);
    return;
  }

  String id = parts[0];

  // Fan is actuator-only; STM32 should not send a "fan" line.
  if (id == "fan") {
    // Ignore silently or log once if you want.
    Serial.println("STM parse: ignoring 'fan' line (actuator only).");
    return;
  }

  LoadState *load = findLoadById(id);
  if (!load) {
    // Unknown id, ignore.
    Serial.print("STM parse: unknown id '");
    Serial.print(id);
    Serial.println("'");
    return;
  }

  // --- on/off ---
  int onInt = parts[1].toInt();
  load->on = (onInt != 0);

  // --- numeric fields if present ---
  if (n >= 3) {
    load->voltage_V = parts[2].toFloat();
  }
  if (n >= 4) {
    load->current_A = parts[3].toFloat();
  }
  if (n >= 5) {
    load->power_W = parts[4].toFloat();
  } else {
    // If STM32 doesn't send power, compute it from V and I.
    load->power_W = load->voltage_V * load->current_A;
  }
  if (n >= 6) {
    load->energy_Wh = parts[5].toFloat();
  }

  // Debug so you can see updates in Serial Monitor
  Serial.print("STM update -> ");
  Serial.print(load->id);
  Serial.print(": on=");
  Serial.print(load->on ? "1" : "0");
  Serial.print(" V=");
  Serial.print(load->voltage_V);
  Serial.print(" I=");
  Serial.print(load->current_A);
  Serial.print(" P=");
  Serial.print(load->power_W);
  Serial.print(" E=");
  Serial.println(load->energy_Wh);
}

void stmInit(HardwareSerial &serialPort, uint32_t baud) {
  stmSerial = &serialPort;
  stmSerial->begin(baud);
}

void stmPoll() {
  if (!stmSerial) return;

  // Process all full lines currently in the UART buffer
  while (stmSerial->available()) {
    String line = stmSerial->readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      continue;
    }
    handleStmLine(line);
  }
}
