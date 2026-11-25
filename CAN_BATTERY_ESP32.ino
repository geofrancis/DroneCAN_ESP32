#include <DroneCAN_ESP32.h>

DroneCAN_ESP32 dronecan;
const uint8_t NODE_ID = 30; // Choose a unique ID for your battery node

// --- BatteryInfo constants (from DroneCAN DSDL) ---
const uint64_t BATTERYINFO_SIGNATURE = 0x249C26548A711966ULL; //
const uint16_t BATTERYINFO_ID = 1092;

// --- NodeStatus constants (standard) ---
const uint64_t NODESTATUS_SIGNATURE = 0xF0868D0C1A7C6F1ULL;
const uint16_t NODESTATUS_ID = 341;

// Fake battery data
float sim_voltage = 48.0f;      // Volts
float sim_current = 5.0f;       // Amperes (discharge)
float sim_temp_c = 25.0f;       // Celsius
uint8_t sim_soc = 85;           // State of Charge %

unsigned long last_send_ms = 0;
const unsigned long SEND_INTERVAL_MS = 1000; // 1 Hz

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("DroneCAN Battery Emulator starting...");

  if (!dronecan.begin(1000000, NODE_ID)) {
    Serial.println("DroneCAN init failed!");
    while (1);
  }
  Serial.println("Node ready. Broadcasting BatteryInfo...");
}

void loop() {
  dronecan.update();

  unsigned long now = millis();
  if (now - last_send_ms >= SEND_INTERVAL_MS) {
    last_send_ms = now;

    // --- 1. Broadcast NodeStatus (required for node discovery) ---
    uavcan_protocol_NodeStatus node_status = {};
    node_status.uptime_sec = millis() / 1000;
    node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;

    uint8_t node_status_buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];
    uint16_t node_status_len = uavcan_protocol_NodeStatus_encode(&node_status, node_status_buffer);
    dronecan.broadcast(NODESTATUS_SIGNATURE, NODESTATUS_ID, CANARD_TRANSFER_PRIORITY_MEDIUM,
                       node_status_buffer, node_status_len);

    // --- 2. Build and broadcast BatteryInfo ---
    uavcan_equipment_power_BatteryInfo battery = {};

    // Primary parameters (Float16 fields)
    battery.temperature = sim_temp_c + 273.15f; // Convert to Kelvin
    battery.voltage = sim_voltage;
    battery.current = sim_current;
    battery.average_power_10sec = sim_voltage * sim_current; // Simple approximation
    battery.remaining_capacity_wh = 500.0f;     // Fake 500 Wh remaining
    battery.full_charge_capacity_wh = 600.0f;   // Fake 600 Wh full capacity
    battery.hours_to_full_charge = 0.0f;        // Not charging

    // Status flags: set as "in use" (bit 0)
    battery.status_flags = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATUS_FLAG_IN_USE;

    // State of health (optional, use unknown constant)
    battery.state_of_health_pct = UAVCAN_EQUIPMENT_POWER_BATTERYINFO_STATE_OF_HEALTH_UNKNOWN;

    // State of charge (required)
    battery.state_of_charge_pct = sim_soc;
    battery.state_of_charge_pct_stdev = 0; // Unknown

    // Identification
    battery.battery_id = 0;                 // Primary battery
    battery.model_instance_id = 0;          // Not applicable
    // Set model name (optional)
    const char* model = "ESP32 Emulated Battery";
    battery.model_name.len = strlen(model);
    memcpy(battery.model_name.data, model, battery.model_name.len);

    // Encode and broadcast
    uint8_t battery_buffer[UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE];
    uint16_t battery_len = uavcan_equipment_power_BatteryInfo_encode(&battery, battery_buffer);

    dronecan.broadcast(BATTERYINFO_SIGNATURE, BATTERYINFO_ID, CANARD_TRANSFER_PRIORITY_LOW,
                       battery_buffer, battery_len);

    Serial.printf("Sent BatteryInfo -> V:%.1f, A:%.1f, SOC:%d%%, Temp:%.1fC\n",
                  sim_voltage, sim_current, sim_soc, sim_temp_c);

    // Vary SOC to simulate discharge
    if (sim_soc > 0) sim_soc--;
    sim_current = 4.0f + (random(0, 200) / 100.0f); // 4.0 - 6.0 A
    sim_voltage = 48.0f - (sim_current * 0.05f);     // Simple voltage sag
  }
}