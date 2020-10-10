#pragma once

#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

#define MCI_CRUISE_CHANGE_AMOUNT_MS (1.0f / 3.6f)

// TODO(SOFT-201): Current is 30mph because that seems to be the standard but should finalize
#define MCI_MIN_CRUISE_VELOCITY_MS 11.0f

#define MCI_MAX_CRUISE_VELOCITY_MS 100.0f

StatusCode cruise_rx_init(void);

void cruise_rx_update_velocity(float current_speed);

float cruise_rx_get_target_velocity(void);