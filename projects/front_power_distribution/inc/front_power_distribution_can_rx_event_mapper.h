#pragma once

// Map CAN messages to events.
// Requires CAN and the event queue to be initialized.

#include "status.h"

StatusCode front_power_distribution_can_rx_event_mapper_init(void);
