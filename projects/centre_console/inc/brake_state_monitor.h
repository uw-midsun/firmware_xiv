#pragma once

// This module listens on CAN for the brake state, and keeps a global variable for the
// brake state. It is used to decide on beginning the power_on and power_off sequence.

#include "status.h"

typedef enum { BRAKE_STATE_PRESSED = 0, BRAKE_STATE_RELEASED, NUM_BRAKE_STATES } BrakeState;

StatusCode brake_state_monitor_init(void);

BrakeState *get_brake_state(void);
