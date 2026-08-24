/*
 * ============================================================
 *  Schneider PM2100 Energy Meter -> ESP32 (Modbus RTU / RS485)
 * ============================================================
 *  Reads live data from a Schneider PM2100 power meter using
 *  an ESP32 and an RS485-to-TTL converter (auto flow-control).
 *
 *  Values read (32-bit float, 2 registers each):
 *    - Voltage A-N
 *    - Voltage B-N
 *    - Voltage C-N
 *    - Frequency
 *
 *  Library: ModbusMaster  ->  https://github.com/4-20ma/ModbusMaster
 *  Channel: IoT Bhai
 * ============================================================
 */

#include <ModbusMaster.h>

/* -------------------- Meter settings -------------------- */
#define METER_ID    1          // Modbus slave address of the PM2100
#define TOTAL_REG   4          // Number of parameters we read

// PM2100 register addresses (each value = 2 registers = 1 float)
#define REG_AN   3027          // Voltage A-N
#define REG_BN   3029          // Voltage B-N
#define REG_CN   3031          // Voltage C-N
#define REG_FQ   3109          // Frequency

uint16_t reg_addr[TOTAL_REG] = {
  REG_AN,
  REG_BN,
  REG_CN,
  REG_FQ,
};

float DATA_METER[TOTAL_REG];   // Latest values read from the meter

/* -------------------- RS485 / Modbus wiring -------------------- */
#define MODBUS_RX_PIN       18         // ESP32 RX  ->  RO (or TXD) of RS485 module
#define MODBUS_TX_PIN       19         // ESP32 TX  ->  DI (or RXD) of RS485 module
#define MODBUS_SERIAL_BAUD  19200      // Must match the meter's baud rate
#define PARITY              SERIAL_8E1 // 8 data bits, Even parity, 1 stop bit

// ModbusMaster object (our node/master)
ModbusMaster node;

/* -------------------- Helpers -------------------- */
// Reinterpret a 32-bit value as an IEEE-754 float
float HexToFloat(uint32_t x) {
  return (*(float*)&x);
}

// Read one float parameter (2 registers) from the meter
float read_meter_float(uint16_t reg) {
  uint16_t data[2];
  uint32_t value = 0;

  uint8_t result = node.readHoldingRegisters(reg, 2);
  delay(500);

  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);   // high word
    data[1] = node.getResponseBuffer(1);   // low word

    value = ((uint32_t)data[0] << 16) | data[1];
    return HexToFloat(value);
  } else {
    Serial.print("Modbus read failed. REG >>> ");
    Serial.println(reg);
    delay(500);
    return 0;
  }
}

// Read all parameters into DATA_METER[]
void get_meter() {
  delay(500);
  for (uint8_t i = 0; i < TOTAL_REG; i++) {
    DATA_METER[i] = read_meter_float(reg_addr[i]);
  }
}

/* -------------------- Setup -------------------- */
void setup() {
  // Serial monitor
  Serial.begin(115200);

  // Serial2 for RS485 communication -> Serial2.begin(baud, protocol, RX, TX)
  Serial2.begin(MODBUS_SERIAL_BAUD, PARITY, MODBUS_RX_PIN, MODBUS_TX_PIN);
  Serial2.setTimeout(200);

  // Bind the Modbus node to the meter address and Serial2 (once)
  node.begin(METER_ID, Serial2);
}

/* -------------------- Loop -------------------- */
void loop() {
  get_meter();

  Serial.println();
  Serial.print("Volt A-N   = "); Serial.print(DATA_METER[0]); Serial.println(" V");
  Serial.print("Volt B-N   = "); Serial.print(DATA_METER[1]); Serial.println(" V");
  Serial.print("Volt C-N   = "); Serial.print(DATA_METER[2]); Serial.println(" V");
  Serial.print("Frequency  = "); Serial.print(DATA_METER[3]); Serial.println(" Hz");

  delay(3000);
}
