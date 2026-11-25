The DroneCAN_ESP32

The ESP32's DroneCAN protocol implementation library is based on [Libcanard] (https://github.com/UAVCAN/libcanard) and [ACAN_ESP32] (https://github.com/seeed-studio/ACAN_ESP32).

#Preliminary

DroneCAN_ESP32 provides complete DroneCAN protocol support for ESP32 microcontrollers, supports broadcast, request/response, and other transport types, and presets a rich definition of UAVCAN message types.

#Functional features

- **Fully DroneCAN protocol stack** — based on Libcanard, UAVCAN V0 protocol support
- ** Flexible transmission type** - support for broadcast, request, response modes
- ** Configurable memory pool** — Supports custom CAN transceiver buffer sizes
- ** Callback mechanism** - Support for custom receive callbacks and transmission filtering callbacks
- **Rich message types** — Preset 50+ common UAVCAN message types, overlay:
  GNSS-related (Fix, RTCMStream, Auxiliary)
  - Power management (BatteryInfo, CircuitStatus, PrimaryPowerSupplyStatus)
  - Lights Command, BeepCommand, RGB565
  - File operations (Read, Write, GetDirectoryEntryInfo)
  Parameter management (GetSet, ExecuteOpcode)
  - Node management (GetNodeInfo, RestartNode)
  Debugging (LogMessage, KeyValue)
  And more...

# of installation

#### with Arduino Library Manager

1. Open the Arduino IDE → **Sketch** → **Include Library** → **Manage Libraries**
2. Search for `DroneCAN_ESP32`
3. Click on Install

## Installed Manually

Clone this repository into your Arduino libraries directory:

"Bash"
CD ~/Arduino/libraries
git clone https://github.com/dukang1996/DroneCAN_ESP32.git
"```

# – Start quickly

### Hardware Connections

"```
ESP32 CAN Transceiver DroneCAN Bus
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  TX – ► TX CAN_H – ► Other nodes
  RX ◄ – RX CAN_L – ► Other nodes
          ┌--VVC (3.3V) 120Ω terminal resistance (both ends of the bus)
          └-GND
"```

**Note: Please connect a 120 Ω terminal resistor at both ends of the bus.

## Basic Examples

"The Cpp"
#include <DroneCAN_ESP32.h>

The DroneCAN_ESP32 dronecan

Receiving the callback
void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer) {
    Serial.print("Received data_type=");
    Serial.print(transfer->data_type_id);
    Serial.print(", length=");
    Serial.println(transfer->payload_len);
}

// Transmission filter callback
bool shouldAcceptTransfer(const CanardInstance* ins,
                         uint64_t* out_data_type_signature,
                         uint16_t data_type_id,
                         CanardTransferType transfer_type,
                         uint8_t source_node_id) {
    if (data_type_id == 123) {
        *out_data_type_signature = 0x1234567890ABCDEF;
