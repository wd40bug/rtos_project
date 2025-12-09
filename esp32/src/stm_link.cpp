// src/stm_link.cpp
#include "stm_link.h"
#include "loads.h"
#include <string.h>
#include <Arduino.h>

// Pointer to the HardwareSerial instance used to talk to the STM32.
// This is set in stmInit() and used everywhere else in this file.
static HardwareSerial *stmSerial = nullptr;

// ---- Relay command queue ----
// Simple ring buffer entry describing a relay command the ESP32
// wants to send to the STM32. "on" is a logical flag from the ESP's
// perspective, but the STM32 may treat any command as a "toggle".
struct RelayCommand {
  int index;  // 0 = lamp, 1 = charger, 2 = fan
  bool on;    // desired logical state (used for logging / bookkeeping)
};

// Fixed-size ring buffer to hold pending relay commands.
static RelayCommand cmdQueue[8];  // small ring buffer
static uint8_t qHead = 0;         // index of next element to pop
static uint8_t qTail = 0;         // index of next free slot to push

// Returns true if there are no pending commands in the queue.
static bool queueIsEmpty() {
  return qHead == qTail;
}

// Returns true if the queue has no free slots left for new commands.
static bool queueIsFull() {
  // Ring buffer is full when advancing tail would collide with head.
  return static_cast<uint8_t>(qTail + 1) % 8 == qHead;
}

// Push a new relay command into the queue to be sent to the STM32 later.
bool stmQueueRelayCommand(int index, bool on) {
  // Validate relay index range (we only have 3 relays)
  if (index < 0 || index > 2) {
    return false;  // invalid index
  }
  // Cannot queue if UART has not been initialized yet.
  if (!stmSerial) {
    return false;  // UART not ready
  }
  // Drop command if queue is currently full.
  if (queueIsFull()) {
    return false;  // drop if full
  }

  // Store command at tail, then advance tail with wrap-around.
  cmdQueue[qTail].index = index;
  cmdQueue[qTail].on = on;
  qTail = static_cast<uint8_t>(qTail + 1) % 8;
  return true;
}

// Pop the oldest relay command from the queue into 'out'.
// Returns true if a command was popped, false if the queue was empty.
static bool popRelayCommand(RelayCommand &out) {
  if (queueIsEmpty()) return false;
  out = cmdQueue[qHead];
  qHead = static_cast<uint8_t>(qHead + 1) % 8;
  return true;
}

// --- helper: split a String on ',' into up to maxParts pieces ---
// Takes an input String line and splits it at commas into outParts[].
// Returns the number of tokens written into outParts.
static int splitCommaSeparated(const String &line, String *outParts, int maxParts) {
  int count = 0;            // how many parts we have filled so far
  int start = 0;            // start index of the current token
  int len = line.length();  // total length of the string

  for (int i = 0; i <= len && count < maxParts; ++i) {
    // When we reach end-of-string or a comma, we cut a token.
    if (i == len || line[i] == ',') {
      outParts[count++] = line.substring(start, i);
      start = i + 1;  // next token starts after the comma
    }
  }
  return count;
}

// --- handle one complete line from STM32 ---
// Expected format (one line per load):
//   lamp,1,120.0,0.25,30.0,12.5
//   charger,1,5.0,1.2,6.0,3.4
//
// Fields:
// 0: id        -> "lamp" or "charger"
// 1: on        -> "0" or "1"
// 2: voltage   -> voltage reading (V)
// 3: current   -> current reading (A)
// 4: power     -> power (W)
// 5: energy    -> energy (Wh), optional
static void handleStmLine(const String &line) {
  // Safety check: if UART is not initialized, bail out.
  if (!stmSerial) return;

  // Echo the raw line to the USB serial for debugging.
  Serial.print(line);

  // Split the CSV line into up to 6 parts.
  String parts[6];
  int n = splitCommaSeparated(line, parts, 6);

  // At minimum we need id and on/off (2 fields).
  if (n < 2) {
    Serial.print("STM parse: too few fields: ");
    Serial.println(line);
    // Still respond with ACK so the STM32 knows we received something
    // and does not stall waiting for a response.
    stmSerial->print("ACK\n");
    Serial.println("Sent ACK (parse error)");
    return;
  }

  // First field is the load identifier string ("lamp", "charger", maybe "fan").
  String id = parts[0];

  if (id == "fan") {
    // Fan is an actuator-only device; STM32 should not report measurements for it.
    // If it does, we ignore the line to avoid corrupting the local model.
    Serial.println("STM parse: ignoring 'fan' line (actuator only).");
  } else {
    // Look up the load in g_loads by its id string.
    LoadState *load = findLoadById(id);
    if (!load) {
      // If the id is unknown, log an error and still send an ACK so the protocol continues.
      Serial.print("STM parse: unknown id '");
      Serial.print(id);
      Serial.println("'");
      stmSerial->print("ACK\n");
    } else {
      // Convert the on/off field into a boolean.
      int onInt = parts[1].toInt();
      load->on = (onInt != 0);

      // Parse voltage, current, power, energy if present.
      if (n >= 3) load->voltage_V = parts[2].toFloat();
      if (n >= 4) load->current_A = parts[3].toFloat();
      if (n >= 5) load->power_W   = parts[4].toFloat();
      else        load->power_W   = load->voltage_V * load->current_A; // fallback if no power field
      if (n >= 6) load->energy_Wh = parts[5].toFloat();

      // Print a summary of the updated load state for debugging.
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
  }

  // ---- Respond to STM: relay command or ACK ----
  RelayCommand cmd;
  if (popRelayCommand(cmd)) {
    // There is a queued command: send it as "index,onInt\n".
    // Note: onInt is a logical flag; on the STM32 side this may be used as
    // absolute on/off or simply ignored if that side treats every message as "toggle".
    int onInt = cmd.on ? 1 : 0;
    stmSerial->printf("%d,%d\n", cmd.index, onInt);  // "index,on/off"
    Serial.printf("Sent relay cmd to STM: %d,%d\n", cmd.index, onInt);
  } else {
    // No pending commands: send ACK so the STM32 knows we processed the line.
    stmSerial->print("ACK\n");
    Serial.println("Sent ACK to STM");
  }
}

// Set up the STM32 serial link by storing the serial port pointer and configuring its baud rate.
void stmInit(HardwareSerial &serialPort, uint32_t baud) {
  stmSerial = &serialPort;
  stmSerial->begin(baud);
}

// Poll the STM32 UART, read any available lines, and process them.
void stmPoll() {
  // If STM32 link has not been initialized, do nothing.
  if (!stmSerial) return;

  // While there is at least one byte available from STM32...
  while (stmSerial->available()) {
    // Read a full line terminated by '\n' from the STM32.
    String line = stmSerial->readStringUntil('\n');
    line.trim();                     // Remove any trailing \r or whitespace
    if (line.length() == 0) continue; // Skip empty lines
    delay(10);                       // Small delay to allow UART buffer to stabilize
    handleStmLine(line);             // Parse and respond to this line
  }
}
