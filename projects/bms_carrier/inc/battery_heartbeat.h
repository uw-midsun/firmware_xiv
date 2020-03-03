#pragma once

#include "can_ack.h"
#include "exported_enums.h"

#define BATTERY_HEARTBEAT_PERIOD_MS 1000
#define BATTERY_HEARTBEAT_MAX_ACK_FAILS 3

typedef struct BatteryHeartbeatStorage {
  uint32_t period_ms;
  uint32_t expected_bitset;
  uint8_t fault_bitset;
  uint8_t ack_fail_counter;
} BatteryHeartbeatStorage;

// Initializes heartbeat
// initialize with: battery_heartbeat_init(BatteryHeartbeatStorage, BATTERY_HEARTBEAT_PERIOD_MS,
// BATTERY_HEARTBEAT_EXPECTED_DEVICES) 
// Sets up period_ms, expected_bitset in storage; resets fault bitset and counter; starts soft timer
StatusCode battery_heartbeat_init(BatteryHeartbeatStorage *storage, uint32_t period_ms,
                                  uint32_t expected_bitset);

// Handles faults, sends to prv_transmit_battery_status if not due to ACK timeout
StatusCode battery_heartbeat_raise_fault(BatteryHeartbeatStorage *storage,
                                         EEBatteryHeartbeatFaultSource source);

// Clears faults
StatusCode battery_heartbeat_clear_fault(BatteryHeartbeatStorage *storage,
                                         EEBatteryHeartbeatFaultSource source);
