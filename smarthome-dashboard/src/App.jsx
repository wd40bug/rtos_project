// src/App.jsx
import React, { useEffect, useState, useMemo } from "react";

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL;

function App() {
  const [status, setStatus] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [lastUpdated, setLastUpdated] = useState(null);

  // ===== POLLING =====
  useEffect(() => {
    let cancelled = false;

    async function fetchStatus() {
      try {
        const res = await fetch(`${API_BASE_URL}/api/status`);
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        const data = await res.json();
        if (cancelled) return;

        setStatus(data);
        setError(null);
        setLastUpdated(new Date());
        setLoading(false);
      } catch (err) {
        console.error("Failed to fetch status:", err);
        if (!cancelled) {
          setError("Lost connection to device");
          setLoading(false);
        }
      }
    }

    fetchStatus();
    const id = setInterval(fetchStatus, 1000);

    return () => {
      cancelled = true;
      clearInterval(id);
    };
  }, []);

  // ===== ACTIONS =====
  async function toggleLoad(loadId, currentOn) {
    // Fan is auto-controlled; ignore manual toggle for now
    if (loadId === "fan") return;

    try {
      const res = await fetch(`${API_BASE_URL}/api/control`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ loadId, on: !currentOn }),
      });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);

      setStatus((prev) => {
        if (!prev) return prev;
        return {
          ...prev,
          loads: {
            ...prev.loads,
            [loadId]: {
              ...prev.loads[loadId],
              on: !currentOn,
            },
          },
        };
      });
      setError(null);
    } catch (err) {
      console.error("Failed to send control command:", err);
      setError("Failed to send command");
    }
  }

  // ===== SUMMARY NUMBERS =====
  const { totalPower, devicesOn, totalDevices, thresholdW } = useMemo(() => {
    const threshold = status?.thresholds?.highUsage_W ?? null;

    if (!status?.loads) {
      return {
        totalPower: status?.totalPower_W ?? 0,
        devicesOn: 0,
        totalDevices: 0,
        thresholdW: threshold,
      };
    }

    const entries = Object.entries(status.loads);
    const monitoredEntries = entries.filter(([id]) => id !== "fan");

    const totalP =
      typeof status?.totalPower_W === "number"
        ? status.totalPower_W
        : monitoredEntries.reduce(
            (sum, [, l]) => sum + (l.power_W || 0),
            0
          );

    const onCount = entries.filter(([, l]) => l.on).length;

    return {
      totalPower: totalP,
      devicesOn: onCount,
      totalDevices: entries.length,
      thresholdW: threshold,
    };
  }, [status]);

  function renderLoadCard(loadId, load) {
    const isFan = loadId === "fan";

    const isHigh =
      !isFan &&
      thresholdW != null &&
      typeof load.power_W === "number" &&
      load.power_W >= thresholdW;

    return (
      <div
        key={loadId}
        style={{
          borderRadius: "1rem",
          padding: "0.9rem 1rem",
          background:
            "radial-gradient(circle at top left, #1f2937, #020617 60%)",
          boxShadow: "0 10px 24px rgba(0,0,0,0.45)",
          display: "flex",
          flexDirection: "column",
          gap: "0.4rem",
        }}
      >
        {/* Header row */}
        <div
          style={{
            display: "flex",
            justifyContent: "space-between",
            alignItems: "center",
            gap: "0.4rem",
          }}
        >
          <h2
            style={{
              margin: 0,
              fontSize: "1.05rem",
            }}
          >
            {load.name}
          </h2>

          <span
            style={{
              fontSize: "0.75rem",
              padding: "0.15rem 0.65rem",
              borderRadius: "999px",
              backgroundColor: load.on ? "#bbf7d0" : "#e5e7eb",
              border: `1px solid ${load.on ? "#22c55e" : "#9ca3af"}`,
              color: load.on ? "#15803d" : "#4b5563",
              fontWeight: 600,
            }}
          >
            {isFan ? (load.on ? "AUTO • ON" : "AUTO • OFF") : load.on ? "ON" : "OFF"}
          </span>
        </div>

        {/* Numbers / description */}
        <div
          style={{
            fontSize: "0.85rem",
            lineHeight: 1.4,
            color: "#d1d5db",
          }}
        >
          {isFan ? (
            <div style={{ fontSize: "0.8rem", color: "#e5e7eb" }}>
              Auto-controlled cooling fan that turns on when total monitored
              power exceeds the threshold.
            </div>
          ) : (
            <>
              <div>Voltage: {load.voltage_V?.toFixed(2)} V</div>
              <div>Current: {load.current_A?.toFixed(3)} A</div>
              <div>
                Power:{" "}
                <strong style={{ color: isHigh ? "#f97316" : "#f9fafb" }}>
                  {load.power_W?.toFixed(1)} W {isHigh && "⚠️"}
                </strong>
              </div>
              <div>Energy: {load.energy_Wh?.toFixed(2)} Wh</div>
            </>
          )}
        </div>

        {/* High-usage note */}
        {!isFan && isHigh && (
          <div
            style={{
              fontSize: "0.75rem",
              color: "#fed7aa",
              backgroundColor: "#7c2d12",
              padding: "0.25rem 0.5rem",
              borderRadius: "0.55rem",
              marginTop: "0.1rem",
            }}
          >
            Above {thresholdW.toFixed(1)} W threshold
          </div>
        )}

        {/* Button */}
        {!isFan && (
          <button
            onClick={() => toggleLoad(loadId, load.on)}
            style={{
              marginTop: "0.45rem",
              padding: "0.4rem 0.8rem",
              borderRadius: "999px",
              border: "none",
              cursor: "pointer",
              fontWeight: 600,
              fontSize: "0.85rem",
              backgroundColor: load.on ? "#ef4444" : "#22c55e",
              color: "white",
            }}
          >
            {load.on ? "Turn OFF" : "Turn ON"}
          </button>
        )}
      </div>
    );
  }

  return (
    <div
      style={{
        fontFamily:
          "system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif",
        minHeight: "100vh",
        background: "radial-gradient(circle at top, #020617, #000 75%)",
        color: "#e5e7eb",
        padding: "1rem 0 1.5rem",
      }}
    >
      <div
        style={{
          width: "100vw",
          padding: "0 1.5rem",
          boxSizing: "border-box",
        }}
      >
        {/* HEADER */}
        <header style={{ marginBottom: "0.6rem" }}>
          <h1
            style={{
              margin: 0,
              fontSize: "1.7rem",
            }}
          >
            Smart Home Energy Dashboard
          </h1>
          <p
            style={{
              margin: "0.3rem 0 0.25rem",
              fontSize: "0.9rem",
              color: "#9ca3af",
            }}
          >
            Desk Lamp • Phone Charger • Auto-Cooling Fan
          </p>
          <div
            style={{
              fontSize: "0.78rem",
              color: "#9ca3af",
            }}
          >
            {loading && "Connecting to device..."}
            {lastUpdated &&
              !loading &&
              `Last updated: ${lastUpdated.toLocaleTimeString()}`}
            {error && (
              <span style={{ color: "#fca5a5" }}> • Error: {error}</span>
            )}
          </div>
        </header>

        {/* SUMMARY + PIE CHART */}
        <section
          style={{
            marginBottom: "1rem",
            padding: "0.7rem 0.9rem 0.8rem",
            borderRadius: "0.9rem",
            background:
              "linear-gradient(135deg, rgba(15,23,42,0.96), rgba(37,99,235,0.9))",
            border: "1px solid rgba(148,163,184,0.5)",
            display: "grid",
            gridTemplateColumns: "minmax(0, 1.2fr) minmax(0, 1.4fr)",
            gap: "1rem",
          }}
        >
          <div
            style={{
              display: "flex",
              flexDirection: "column",
              gap: "0.4rem",
              justifyContent: "center",
            }}
          >
            <SummaryItem
              label="Total Monitored Power"
              value={`${totalPower.toFixed(1)} W`}
            />
            <SummaryItem
              label="Devices On"
              value={`${devicesOn} / ${totalDevices}`}
            />
            <SummaryItem
              label="High-usage Threshold"
              value={
                thresholdW != null ? `${thresholdW.toFixed(1)} W` : "Not set"
              }
            />
          </div>
          <div>
            <h2
              style={{
                margin: "0 0 0.25rem",
                fontSize: "0.85rem",
                fontWeight: 500,
                color: "#cbd5f5",
              }}
            >
              Power Distribution (Monitored Loads)
            </h2>
            <PowerPieChart loads={status?.loads} />
          </div>
        </section>

        {/* LOAD CARDS */}
        <section
          style={{
            display: "grid",
            gridTemplateColumns: "repeat(auto-fit, minmax(230px, 1fr))",
            gap: "1rem",
            marginBottom: "1rem",
          }}
        >
          {status &&
            status.loads &&
            Object.entries(status.loads).map(([id, load]) =>
              renderLoadCard(id, load)
            )}
        </section>

        {/* ALERTS */}
        <section
          style={{
            borderRadius: "0.9rem",
            padding: "0.8rem 1rem 0.9rem",
            background:
              "linear-gradient(135deg, #020617, #020617 60%, #111827)",
            border: "1px solid #1f2937",
          }}
        >
          <h2 style={{ marginTop: 0, fontSize: "0.95rem" }}>Alerts</h2>
          {status?.alerts?.length ? (
            <ul
              style={{
                margin: 0,
                paddingLeft: "1.2rem",
                fontSize: "0.85rem",
                color: "#e5e7eb",
              }}
            >
              {status.alerts.map((a, idx) => (
                <li key={idx}>
                  <strong>{a.loadId}</strong>: {a.message}{" "}
                  <span style={{ color: "#9ca3af" }}>
                    ({new Date(a.timestamp * 1000).toLocaleTimeString()})
                  </span>
                </li>
              ))}
            </ul>
          ) : (
            <div
              style={{
                fontSize: "0.82rem",
                color: "#6b7280",
              }}
            >
              No active alerts. Your electrons are behaving.
            </div>
          )}
        </section>
      </div>
    </div>
  );
}

function SummaryItem({ label, value }) {
  return (
    <div>
      <div
        style={{
          fontSize: "0.75rem",
          textTransform: "uppercase",
          letterSpacing: "0.08em",
          color: "#cbd5f5",
          marginBottom: "0.12rem",
        }}
      >
        {label}
      </div>
      <div
        style={{
          fontSize: "1rem",
          fontWeight: 600,
        }}
      >
        {value}
      </div>
    </div>
  );
}

// ===== PIE CHART COMPONENT =====
function PowerPieChart({ loads }) {
  if (!loads) {
    return (
      <PieShell>
        <span style={{ fontSize: "0.8rem", color: "#9ca3af" }}>
          Waiting for data...
        </span>
      </PieShell>
    );
  }

  const entries = Object.entries(loads)
    .filter(([id]) => id !== "fan") // exclude actuator-only fan
    .map(([id, l]) => ({
      id,
      name: l.name || id,
      power: Math.max(0, l.power_W || 0),
    }));

  const total = entries.reduce((s, e) => s + e.power, 0);

  if (total <= 0.0001) {
    return (
      <PieShell>
        <span style={{ fontSize: "0.8rem", color: "#9ca3af" }}>
          Monitored loads at 0 W
        </span>
      </PieShell>
    );
  }

  const colors = {
    lamp: "#f97316", // orange
    charger: "#22c55e", // green
  };

  const cx = 50;
  const cy = 50;
  const r = 40;
  let startAngle = -Math.PI / 2; // start at top

  const slices = entries.map((e) => {
    const fraction = e.power / total;
    const angle = fraction * Math.PI * 2;
    const endAngle = startAngle + angle;

    const x1 = cx + r * Math.cos(startAngle);
    const y1 = cy + r * Math.sin(startAngle);
    const x2 = cx + r * Math.cos(endAngle);
    const y2 = cy + r * Math.sin(endAngle);

    const largeArc = angle > Math.PI ? 1 : 0;

    const pathData = [
      `M ${cx} ${cy}`,
      `L ${x1} ${y1}`,
      `A ${r} ${r} 0 ${largeArc} 1 ${x2} ${y2}`,
      "Z",
    ].join(" ");

    startAngle = endAngle;
    return {
      pathData,
      color: colors[e.id] || "#e5e7eb",
      label: e.name,
      value: e.power,
      fraction,
      key: e.id,
    };
  });

  return (
    <PieShell>
      <svg viewBox="0 0 100 100" style={{ width: "100%", height: "100%" }}>
        {slices.map((s) => (
          <path
            key={s.key}
            d={s.pathData}
            fill={s.color}
            stroke="#020617"
            strokeWidth="0.5"
          />
        ))}
        {/* center hole look */}
        <circle cx={cx} cy={cy} r={18} fill="#020617" />
        <text
          x={cx}
          y={cy - 2}
          textAnchor="middle"
          fontSize="7"
          fill="#e5e7eb"
        >
          {total.toFixed(1)} W
        </text>
        <text
          x={cx}
          y={cy + 7}
          textAnchor="middle"
          fontSize="4"
          fill="#9ca3af"
        >
          total
        </text>
      </svg>

      {/* legend */}
      <div
        style={{
          position: "absolute",
          right: "0.4rem",
          top: "0.4rem",
          display: "flex",
          flexDirection: "column",
          gap: "0.15rem",
          fontSize: "0.75rem",
        }}
      >
        {slices.map((s) => (
          <div
            key={s.key}
            style={{ display: "flex", alignItems: "center", gap: "0.25rem" }}
          >
            <span
              style={{
                width: "10px",
                height: "10px",
                borderRadius: "999px",
                backgroundColor: s.color,
              }}
            />
            <span>
              {s.label}: {s.value.toFixed(1)} W (
              {(s.fraction * 100).toFixed(0)}%)
            </span>
          </div>
        ))}
      </div>
    </PieShell>
  );
}

function PieShell({ children }) {
  return (
    <div
      style={{
        height: "150px",
        borderRadius: "0.7rem",
        backgroundColor: "rgba(15,23,42,0.9)",
        border: "1px solid rgba(30,64,175,0.7)",
        position: "relative",
        display: "flex",
        alignItems: "center",
        justifyContent: "center",
        padding: "0.25rem 0.5rem",
        overflow: "hidden",
      }}
    >
      {children}
    </div>
  );
}

export default App;
