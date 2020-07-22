#pragma once

#include <stdint.h>

#include "debouncer.h"
#include "status.h"

#include "bps_heartbeat.h"
#include "cell_sense.h"
#include "current_sense.h"
#include "fan_control.h"
#include "relay_control_fsm.h"

typedef struct BmsStorage {
  RelayStorage relay_storage;
  CurrentReadings current_readings;
  AfeReadings afe_readings;
  FanStorage fan_storage;
  DebouncerStorage killswitch_storage;
  BpsStorage bps_storage;
} BmsStorage;

StatusCode fault_bps(uint8_t fault_bitmask, bool clear);
