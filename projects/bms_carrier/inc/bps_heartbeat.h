#pragma once

#include <stdint.h>

#include "status.h"

#define BPS_HB_FREQ_MS 1000;

typedef struct BpsStorage {
  uint8_t ack_fail_count;
  uint32_t ack_devices;
} BpsStorage;

StatusCode bps_heartbeat_init(BpsStorage *storage);
