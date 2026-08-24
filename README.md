# Schneider PM2100 Energy Meter → ESP32 (Modbus RTU / RS485)

Read **live electrical data** from a **Schneider PM2100** power meter using an **ESP32** and an **RS485-to-TTL** converter over **Modbus RTU**.

The sketch polls the meter every 3 seconds and prints the values to the Serial Monitor:

- Voltage A‑N
- Voltage B‑N
- Voltage C‑N
- Frequency

> 📺 Project by the **IoT Bhai** YouTube channel.

---

## 🧰 Hardware Required

| Component | Notes |
|---|---|
| ESP32 dev board | Any ESP32 (uses hardware `Serial2`) |
| Schneider PM2100 | Power / energy meter with Modbus RTU |
| RS485 to TTL module | Auto flow-control type recommended (e.g. based on MAX485 with auto-direction, or an auto-DE/RE board) |
| Jumper wires + 120 Ω terminating resistor | Resistor across A/B on long runs |

---

## 🔌 Wiring

**ESP32 ↔ RS485 module**

| ESP32 Pin | RS485 Module | Purpose |
|---|---|---|
| GPIO **18** (RX) | **RO** | Receive from meter |
| GPIO **19** (TX) | **DI** | Transmit to meter |
| 3V3 / 5V | VCC | Power (check your module) |
| GND | GND | Common ground |

**RS485 module ↔ PM2100 meter**

| RS485 Module | PM2100 Terminal |
|---|---|
| **A / D+** | A (+) |
| **B / D-** | B (−) |
| GND (optional) | Shield / common |

> ⚠️ Match A→A and B→B. If communication fails, try swapping A/B first.

---

## ⚙️ Communication Settings

These **must match the settings configured on the PM2100 meter**:

| Setting | Value |
|---|---|
| Slave / Meter ID | `1` |
| Baud rate | `19200` |
| Parity / Frame | `8E1` (8 data, Even parity, 1 stop) |
| Protocol | Modbus RTU |

Change them at the top of the sketch if your meter is set differently:

```cpp
#define METER_ID            1
#define MODBUS_SERIAL_BAUD  19200
#define PARITY              SERIAL_8E1
```

---

## 📇 Modbus Register Map

Each value is a **32-bit IEEE-754 float** spanning **2 registers**, read as **Holding Registers**.

| Parameter | Register |
|---|---|
| Voltage A‑N | `3027` |
| Voltage B‑N | `3029` |
| Voltage C‑N | `3031` |
| Frequency | `3109` |

> ℹ️ Register numbers follow the PM2100 documentation. Depending on your Modbus library/tool, you may need to apply the usual −1 offset for zero-based addressing (this sketch's `ModbusMaster` uses the addresses as-is and works for these registers).

---

## 📚 Dependencies

- [**ModbusMaster**](https://github.com/4-20ma/ModbusMaster) by Doc Walker

Install it via the Arduino IDE:
**Tools → Manage Libraries… → search "ModbusMaster" → Install**

---

## ▶️ Getting Started

1. Wire the ESP32, RS485 module, and PM2100 as shown above.
2. Install the **ModbusMaster** library.
3. Open `schneider_pm2100_data_extraction_with_esp32.ino` in the Arduino IDE.
4. Select your **ESP32 board** and the correct **COM port**.
5. Confirm the meter's Modbus settings match the sketch.
6. Upload the sketch.
7. Open the **Serial Monitor** at **115200 baud**.

### Expected Output

```
Volt A-N   = 230.15 V
Volt B-N   = 229.87 V
Volt C-N   = 231.02 V
Frequency  = 50.00 Hz
```

---

## 🛠️ Troubleshooting

| Symptom | Likely Cause / Fix |
|---|---|
| `Modbus read failed. REG >>> ...` | Wrong baud/parity, wrong meter ID, or A/B swapped |
| All zeros | No response from meter — check wiring & power |
| Garbage / unstable values | Add 120 Ω termination; verify common ground |
| Wrong readings | Confirm register addresses match your meter firmware |

---

## 📝 How It Works

- `Serial2` is initialized with the meter's baud rate and `8E1` frame.
- `ModbusMaster` reads 2 holding registers per parameter.
- The two 16-bit words are combined (`high << 16 | low`) and reinterpreted as an IEEE-754 float.
- All parameters are refreshed each loop and printed to Serial every 3 seconds.

---

## 📄 License

Released under the **MIT License**. Free to use, modify, and share.

---

*Made with ⚡ by IoT Bhai — if this helped you, consider subscribing!*
