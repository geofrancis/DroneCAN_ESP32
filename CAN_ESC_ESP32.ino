#include <DroneCAN_ESP32.h>

DroneCAN_ESP32 dronecan;
const uint8_t NODE_ID = 10;

// --- Message Signatures and IDs (from DroneCAN standard) ---
// NodeStatus (uavcan.protocol.NodeStatus)
const uint64_t NODESTATUS_SIGNATURE = 0xF0868D0C1A7C6F1ULL; //
const uint16_t NODESTATUS_ID = 341; //[reference:3]

// ESC Status (uavcan.equipment.esc.Status)
// Note: The exact signature value can be found in the library's header files
// or DroneCAN DSDL definitions. The ID is typically around 1030.
const uint64_t ESC_STATUS_SIGNATURE = 0x0000000000000000ULL; // REPLACE with actual value
const uint16_t ESC_STATUS_ID = 1030; // REPLACE with actual value if different

// Synthetic data
float sim_voltage = 12.6f;
float sim_current = 5.0f;
int32_t sim_rpm = 5000;
float sim_temp_k = 298.15f;

unsigned long last_send_ms = 0;
const unsigned long SEND_INTERVAL_MS = 1000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("DroneCAN ESC Sender starting...");

  if (!dronecan.begin(1000000, NODE_ID)) {
    Serial.println("DroneCAN init failed!");
    while (1);
  }
  Serial.println("DroneCAN node initialized.");
}

void loop() {
  dronecan.update();

  unsigned long now = millis();
  if (now - last_send_ms >= SEND_INTERVAL_MS) {
    last_send_ms = now;

    // --- 1. Broadcast NodeStatus ---
    uavcan_protocol_NodeStatus node_status = {};
    node_status.uptime_sec = millis() / 1000;
    node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;

    uint8_t node_status_buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];
    uint16_t node_status_len = uavcan_protocol_NodeStatus_encode(&node_status, node_status_buffer);

    dronecan.broadcast(NODESTATUS_SIGNATURE, NODESTATUS_ID, CANARD_TRANSFER_PRIORITY_MEDIUM,
                       node_status_buffer, node_status_len);

    // --- 2. Broadcast ESC Status ---
    uavcan_equipment_esc_Status esc_status = {};
    esc_status.esc_index = 0;
    esc_status.voltage = sim_voltage;
    esc_status.current = sim_current;
    esc_status.temperature = sim_temp_k;
    esc_status.rpm = sim_rpm;
    esc_status.power_rating_pct = 50;

    uint8_t esc_buffer[UAVCAN_EQUIPMENT_ESC_STATUS_MAX_SIZE];
    uint16_t esc_len = uavcan_equipment_esc_Status_encode(&esc_status, esc_buffer);

    dronecan.broadcast(ESC_STATUS_SIGNATURE, ESC_STATUS_ID, CANARD_TRANSFER_PRIORITY_LOW,
                       esc_buffer, esc_len);

    Serial.printf("Sent ESC Status -> V:%.1f, A:%.1f, RPM:%d\n", sim_voltage, sim_current, sim_rpm);

    // Vary data
    sim_rpm += 100;
    if (sim_rpm > 6000) sim_rpm = 4000;
    sim_current = 5.0f + (random(0, 100) / 100.0f);
    sim_voltage = 12.6f - (sim_current * 0.1f);
  }
}