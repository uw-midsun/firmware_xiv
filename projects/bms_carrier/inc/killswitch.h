#pragma once
// Handles the killswitch
// Requires GPIO, GPIO interrupts, and interrupts to be initialized.
//
// Raises a fault event if the killswitch is hit as an input.
// Allows bypassing the killswitch as an output.
#include "battery_heartbeat.h"
#include "debouncer.h"
#include "gpio.h"

typedef struct KillswitchStorage {
  DebouncerStorage debouncer;
} KillswitchStorage;

// typedef enum {
//   EE_BATTERY_HEARTBEAT_FAULT_SOURCE_KILLSWITCH = 0,
//   // EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_CELL,
//   // EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_TEMP,
//   // EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_FSM,
//   // EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_ADC,
//   // EE_BPS_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT,
//   // NUM_EE_BPS_HEARTBEAT_FAULT_SOURCES,
// } EEBatteryHeartbeatFaultSource;

// Set the killswitch up to fault if hit. Assumes the killswitch is active-low.
StatusCode killswitch_init(KillswitchStorage *storage, const GpioAddress *killswitch,
                           BatteryHeartbeatStorage *battery_heartbeat);

// Bypass the killswitch. It does not need to be initialized.
StatusCode killswitch_bypass(const GpioAddress *killswitch);
