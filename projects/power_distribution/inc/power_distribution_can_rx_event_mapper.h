#pragma once

// Map CAN messages to events.
// Requires CAN and the event queue to be initialized.

// NOT YET GENERIC

#include "status.h"

StatusCode power_distribution_can_rx_event_mapper_init(void);
