// src/main.cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "loads.h"
#include "stm_link.h"

// ================== CONFIG ==================
const char *WIFI_SSID = "";
const char *WIFI_PASS = "";

float HIGH_USAGE_W = 36.0f;

// UART pins for STM32 (adjust to your wiring)
const int STM32_RX_PIN = 16;  // ESP32 receives from STM32 TX
const int STM32_TX_PIN = 17;  // ESP32 sends to STM32 RX (optional)

// Set true once you actually have STM32 sending data and parsing is done
bool USE_STM32_UART = false;

// ================== GLOBALS ==================
WebServer server(80);

// ================== HELPERS ==================
void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ================== HTTP HANDLERS ==================

void handleStatus() {
  addCORS();

  // --- Update loads from STM32 or fake data ---
  if (USE_STM32_UART) {
    stmPoll();          // pull in fresh data from STM32 (will update g_loads)
  } else {
    updateFakeLoads();  // keep things alive until STM32 is ready
  }

  // --- Automatic fan control based on total monitored power ---
  float totalPower = getTotalPowerW();  // lamp + charger only (fan excluded in loads.cpp)
  LoadState *fan = findLoadById("fan");
  if (fan) {
    // Simple hysteresis so it doesn't chatter on/off around the threshold
    static bool fanWasOn = false;

    if (!fanWasOn && totalPower > HIGH_USAGE_W) {
      fan->on = true;
      fanWasOn = true;
      Serial.println("Fan AUTO -> ON (high total power)");
      // TODO: if STM32 actually drives the relay, send a UART command here, e.g.:
      // Serial2.println("fan,1");
    } else if (fanWasOn && totalPower < HIGH_USAGE_W * 0.8f) {
      fan->on = false;
      fanWasOn = false;
      Serial.println("Fan AUTO -> OFF (power normalized)");
      // Serial2.println("fan,0");
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
    o["voltage_V"] = l.voltage_V;
    o["current_A"] = l.current_A;
    o["power_W"] = l.power_W;
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

// POST /api/control  { "loadId":"fan", "on":true }
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

  // NOTE: auto-control logic in handleStatus() will still enforce the threshold;
  // manual fan toggles from the UI can be overridden by the auto logic on next poll.
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

// ================== SETUP & LOOP ==================
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

  // If you want STM32 updates more often, you can also poll here:
  if (USE_STM32_UART) {
    stmPoll();
  }
}
