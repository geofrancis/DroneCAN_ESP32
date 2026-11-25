#include <DroneCAN_ESP32.h>

DroneCAN_ESP32 dronecan;

// Node ID for this receiver
const uint8_t NODE_ID = 20;

// Receive callback – now prints every message, no filtering
void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer) {
  Serial.printf("\n[DroneCAN] type_id=%u  src_node=%u  len=%u bytes\n",
                transfer->data_type_id,
                transfer->source_node_id,
                transfer->payload_len);

  // Print the raw payload as hex bytes
  Serial.print("  Payload: ");
  for (uint16_t i = 0; i < transfer->payload_len; i++) {
    Serial.printf("%02X ", transfer->payload_head[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("DroneCAN All-Message Printer starting...");

  // Set the receive callback – this will fire for every valid transfer
  dronecan.setReceiveCallback(onTransferReceived);

  // No accept callback is set, so the library should accept all transfers.
  // If your version of the library has a default filter, you may need to add
  // an accept callback that returns true for all messages.

  if (!dronecan.begin(1000000, NODE_ID)) {
    Serial.println("DroneCAN init failed!");
    while (1);
  }
  Serial.println("Listening for all DroneCAN messages at 1 Mbps...");
}

void loop() {
  dronecan.update();
}