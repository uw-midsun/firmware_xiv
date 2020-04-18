#pragma once

#include "can.h"
#include "can_msg_defs.h"
#include "status.h"
#include "watchdog.h"

typedef enum { SPEED_STATE_STATIONARY = 0, SPEED_STATE_MOVING, NUM_SPEED_STATES } SpeedState;

#define STATIONARY_VELOCITY_THRESHOLD 100  // units: cm/s (3.6 km/h)

StatusCode speed_monitor_init(WatchdogTimeout timeout);

SpeedState *get_global_speed_state(void);
