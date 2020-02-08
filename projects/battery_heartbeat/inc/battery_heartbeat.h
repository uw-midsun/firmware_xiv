#pragma once 

#include "can_ack.h" 
//from ms12 code 
#define BATTERY_HEARTBEAT_PERIOD_MS 1000
#define BATTERY_HEARTBEAT_MAX_ACK_FAILS 3
//update with actual pedal name when found (?)
#define BATTERY_HEARTBEAT_EXPECTED_DEVICES
  CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL)

typedef struct BatteryHeartbeatStorage {
  unit32_t period_ms;
  unit32_t expected_bitset; 
  uni8_t fault_bitset; 
  uni8_t ack_fail_counter; 
} BatteryHeartbeatStorage; 

//stole this from previous exported_enums file, should add to current one 

typedef enum {
  EE_BATTERY_HEARTBEAT_FAULT_SOURCE_KILLSWITCH = 0,
  EE_BATTERY_HEARTBEAT_FAULT_SOURCE_LTC_AFE_CELL,
  EE_BATTERY_HEARTBEAT_FAULT_SOURCE_LTC_AFE_TEMP,
  EE_BATTERY_HEARTBEAT_FAULT_SOURCE_LTC_AFE_FSM,
  EE_BATTERY_HEARTBEAT_FAULT_SOURCE_LTC_ADC,
  EE_BATTERY_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT,
  NUM_EE_BATTERY_HEARTBEAT_FAULT_SOURCES,
} EEBatteryHeartbeatFaultSource;

StatusCode battery_heartbeat_init(); //update 

StatusCode battery_heartbeat_raise_fault(); //update 

StatusCode battery_heartbeat_clear_fault(); //update
