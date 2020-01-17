#pragma once

// Module for the throttle

#include <stdint.h>

#include "ads1015.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"

// Time period between every throttle pedal reading
#define THROTTLE_UPDATE_PERIOD_MS 10

// Stores data of the throttle in position_raw, unmapped for now, will figure out mapping later
typedef struct ThrottleStorage {
  Ads1015Channel ads_channel;
  Ads1015Storage ads_storage;
} ThrottleStorage;

StatusCode throttle_init(ThrottleStorage *throttle_storage);

// Enables the throttle
StatusCode throttle_enable(ThrottleStorage *storage);

// Disables the throttle
StatusCode throttle_disable(ThrottleStorage *storage);
