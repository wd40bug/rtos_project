// src/main.cpp

#include <Arduino.h>          // Core Arduino API for ESP32
#include <WiFi.h>             // Wi-Fi control for ESP32
#include <WebServer.h>        // Simple HTTP server implementation
#include <ArduinoJson.h>      // JSON encoding/decoding for HTTP responses

#include "loads.h"            // Declares LoadState array and helper functions for loads
#include "stm_link.h"         // Declares STM32 UART link helpers (stmInit, stmPoll, stmQueueRelayCommand)

// Wifi Configuration
const char *WIFI_SSID = "";   // Wi-Fi network SSID to connect to
const char *WIFI_PASS = "";    // Wi-Fi password

// High Usage Threshold
float HIGH_USAGE_W = 10.0f;             // Threshold (in watts) at which the fan should turn on automatically

// UART pins for STM32 (adjust to your wiring)
const int STM32_RX_PIN = 16;  // ESP32 RX pin (receives data from STM32 TX)
const int STM32_TX_PIN = 17;  // ESP32 TX pin (sends data to STM32 RX)

// Set true to not use fake data (Actual STM connection)
bool USE_STM32_UART = true;  // When true, poll STM32 over UART; when false, use fake data generator

// Global variables
WebServer server(80);         // HTTP server listening on port 80

// Integrate Power over time for Energy in Wh
void updateEnergyFromPower() {
  static unsigned long lastMs = 0;   // Stores the last time this function ran (in ms since boot)
  unsigned long now = millis();      // Current time in ms since boot

  if (lastMs == 0) {
    // initializes timestamp
    lastMs = now;
    return;
  }

  unsigned long dtMs = now - lastMs; // Change in time
  lastMs = now;

  if (dtMs == 0) return;

  // Convert ms -> hours
  float dtHours = dtMs / 3600000.0f;

  for (int i = 0; i < LOAD_COUNT; ++i) {
    LoadState &l = g_loads[i];

    // Skips fan
    if (strcmp(l.id, "fan") == 0) {
      continue;
    }

    // integrate when on
    if (!l.on) continue;

    l.energy_Wh += l.power_W * dtHours;
  }
}

// Helper functions
void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// HTTP Handlers

void handleStatus() {
  addCORS();

  // Update loads from STM32 or fake data
  if (USE_STM32_UART) {
    stmPoll();          // pull in fresh data from STM32 (will update g_loads)
  } else {
    updateFakeLoads();  // keep things alive until STM32 is ready
  }

  // Integrate Power
  updateEnergyFromPower();

  // Automatic fan control based on total monitored power
  float totalPower = getTotalPowerW();  // lamp + charger only (fan excluded in loads.cpp)
  LoadState *fan = findLoadById("fan");
  if (fan) {
    // Simple hysteresis so it doesn't chatter on/off around the threshold
    static bool fanWasOn = false;

    // HIGH edge: fan OFF -> ON
    if (!fanWasOn && totalPower > HIGH_USAGE_W) {
      // queue one toggle to turn fan ON
      if (USE_STM32_UART) {
        stmQueueRelayCommand(2, true);   // 'true' is just semantic; STM toggles
      }

      fanWasOn = true;
      fan->on = true;
      Serial.println("Fan AUTO -> ON (high total power)");
    }
    // LOW edge with hysteresis: fan ON -> OFF
    else if (fanWasOn && totalPower < HIGH_USAGE_W * 0.8f) {
      // queue one toggle to turn fan OFF
      if (USE_STM32_UART) {
        stmQueueRelayCommand(2, false);  // value again doesn't matter to STM
      }

      fanWasOn = false;
      fan->on = false;
      Serial.println("Fan AUTO -> OFF (power normalized)");
    }
  }

  StaticJsonDocument<1024> doc;
  doc["timestamp"] = millis() / 1000;
  doc["totalPower_W"] = totalPower;  // nice to have for the dashboard

  JsonObject loadsObj = doc.createNestedObject("loads");

  for (int i = 0; i < LOAD_COUNT; ++i) {
    LoadState &l = g_loads[i];
    JsonObject o = loadsObj.createNestedObject(l.id);
    o["name"] = l.name;
    o["on"] = l.on;
    o["voltage_V"] = l.voltage_V / 1000;
    o["current_A"] = l.current_A / 1000;
    o["power_W"] = l.power_W / 1000;
    o["energy_Wh"] = l.energy_Wh;
  }

  JsonObject thresholds = doc.createNestedObject("thresholds");
  thresholds["highUsage_W"] = HIGH_USAGE_W;

  JsonArray alerts = doc.createNestedArray("alerts");

  // Alert when total monitored load exceeds threshold (regardless of fan power)
  if (totalPower > HIGH_USAGE_W) {
    JsonObject a = alerts.createNestedObject();
    a["type"] = "HIGH_USAGE_TOTAL";
    a["loadId"] = "total";
    a["message"] = "Total monitored power above threshold; fan may be active";
    a["timestamp"] = millis() / 1000;
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// POST /api/control  { "loadId":"lamp", "on":false }
void handleControl() {
  addCORS();

  String body = server.arg("plain");
  if (body.isEmpty()) {
    server.send(400, "application/json", "{\"error\":\"Empty body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, body)) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  String loadId = doc["loadId"].as<String>();
  bool on = doc["on"];

  LoadState *load = findLoadById(loadId);
  if (!load) {
    server.send(400, "application/json", "{\"error\":\"Unknown loadId\"}");
    return;
  }

  load->on = on;

  // Map loadId -> relay index: lamp=0, charger=1, fan=2
  int relayIndex = -1;
  if (loadId == "lamp") {
    relayIndex = 0;
  } else if (loadId == "charger") {
    relayIndex = 1;
  } else if (loadId == "fan") {
    relayIndex = 2;
  }

  // Queue relay command for STM, if using UART
  if (USE_STM32_UART && relayIndex >= 0) {
    bool ok = stmQueueRelayCommand(relayIndex, on);
    if (!ok) {
      Serial.println("Warning: relay command queue full or UART not ready");
    } else {
      Serial.printf(
        "Queued relay cmd from web: loadId=%s index=%d on=%d\n",
        loadId.c_str(),
        relayIndex,
        on ? 1 : 0
      );
    }
  }

  Serial.printf("Set %s to %s (manual request)\n", loadId.c_str(), on ? "ON" : "OFF");

  StaticJsonDocument<128> resp;
  resp["ok"] = true;
  resp["loadId"] = loadId;
  resp["on"] = on;
  String json;
  serializeJson(resp, json);
  server.send(200, "application/json", json);
}

void handleOptions() {
  addCORS();
  server.send(204, "text/plain", "");
}

void handleNotFound() {
  addCORS();
  server.send(404, "text/plain", "Not found");
}

// Setup and loop
void setup() {
  Serial.begin(115200);
  delay(500);

  randomSeed(analogRead(0));  // for fake noise

  initLoads();

  // STM32 UART init (we pass Serial2 into stmInit)
  Serial2.begin(115200, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);
  stmInit(Serial2, 115200);

  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // HTTP routes
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/control", HTTP_POST, handleControl);
  server.on("/api/control", HTTP_OPTIONS, handleOptions);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  if (USE_STM32_UART) {
    stmPoll();
  }
}
