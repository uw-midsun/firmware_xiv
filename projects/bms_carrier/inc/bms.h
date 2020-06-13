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
  CurrentStorage current_storage;
  AfeStorage afe_storage;
  FanStorage fan_storage;
  DebounceStorage killswitch_storage;
  BpsStorage bps_storage;
  uint8_t fault_bitset;
} BmsStorage;

StatusCode fault_bps(uint8_t fault_bitmask);
