// server/server.js
const express = require("express");
const cors = require("cors");

const app = express();
const PORT = 4000;

app.use(cors());
app.use(express.json());

// In-memory "device" state
let state = {
  timestamp: Math.floor(Date.now() / 1000),
  loads: {
    lamp: {
      name: "Desk Lamp",
      on: true,
      voltage_V: 120.0,
      current_A: 0.25,
      power_W: 30.0,
      energy_Wh: 12.5,
    },
    charger: {
      name: "Phone Charger",
      on: true,
      voltage_V: 5.0,
      current_A: 1.2,
      power_W: 6.0,
      energy_Wh: 3.4,
    },
    fan: {
      name: "Desk Fan",
      on: false,
      voltage_V: 120.0,
      current_A: 0.0,
      power_W: 0.0,
      energy_Wh: 20.1,
    },
  },
  thresholds: {
    highUsage_W: 50,
  },
  alerts: [],
};

// Helper to fake “live” changes
function updateFakeReadings() {
  state.timestamp = Math.floor(Date.now() / 1000);

  // Make the numbers wiggle a bit so it looks alive
  for (const [id, load] of Object.entries(state.loads)) {
    if (!load.on) {
      load.current_A = 0;
      load.power_W = 0;
      continue;
    }

    // Add small random noise
    const noise = (Math.random() - 0.5) * 0.05; // -0.025 to +0.025
    load.current_A = Math.max(0, load.current_A * (1 + noise));
    load.power_W = load.voltage_V * load.current_A;
    load.energy_Wh += load.power_W / 3600; // super rough: +power * seconds / 3600
  }

  // Example alert: if fan > threshold
  state.alerts = [];
  const fan = state.loads.fan;
  if (fan.on && fan.power_W > state.thresholds.highUsage_W) {
    state.alerts.push({
      type: "HIGH_USAGE",
      loadId: "fan",
      message: `Fan above ${state.thresholds.highUsage_W} W`,
      timestamp: state.timestamp,
    });
  }
}

// GET /api/status → pretend this is coming from ESP32
app.get("/api/status", (req, res) => {
  updateFakeReadings();
  res.json(state);
});

// POST /api/control → toggle loads
app.post("/api/control", (req, res) => {
  const { loadId, on } = req.body;

  if (!state.loads[loadId]) {
    return res.status(400).json({ error: "Unknown loadId" });
  }

  state.loads[loadId].on = !!on;
  console.log(`Set ${loadId} to`, on ? "ON" : "OFF");

  // Immediately update readings after change
  updateFakeReadings();

  res.json({ ok: true, state });
});

// Start server
app.listen(PORT, () => {
  console.log(`Fake ESP server listening on http://localhost:${PORT}`);
});
