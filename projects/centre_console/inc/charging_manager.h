#pragma once

#include "status.h"

typedef enum {
  CHARGING_STATE_CHARGING = 0,
  CHARGING_STATE_NOT_CHARGING,
  NUM_CHARGING_STATES
} ChargingState;

StatusCode init_charging_manager(void);

ChargingState get_charging_state(void);
