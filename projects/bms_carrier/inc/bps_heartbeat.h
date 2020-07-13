#pragma once

// Module to periodically send heartbeats and receive the ack

// Requires soft timers and CAN to be initialized

#include <stdint.h>

#include "status.h"

#define BPS_HB_FREQ_MS 1000
#define BPS_ACK_MAX_FAILS 3

typedef struct BpsStorage {
  uint8_t fault_bitset;
  uint8_t ack_fail_count;
  uint32_t ack_devices;
} BpsStorage;

StatusCode bps_heartbeat_init(BpsStorage *storage, uint32_t hb_freq_ms);
