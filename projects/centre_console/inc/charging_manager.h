#pragma once

// This module keeps state of the current state of charging.
// It also resolves charging permissions to the charger board.

#include "drive_fsm.h"
#include "status.h"

typedef enum {
  CHARGING_STATE_CHARGING = 0,
  CHARGING_STATE_NOT_CHARGING,
  NUM_CHARGING_STATES
} ChargingState;

StatusCode init_charging_manager(const DriveState *drive_state);

ChargingState get_global_charging_state(void);
