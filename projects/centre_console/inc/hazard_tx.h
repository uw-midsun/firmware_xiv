#pragma once

// A simple module to keep track of the hazard state and TX it over CAN when it changes.
// Requires the event queue, interrupts, soft timers, gpio, and CAN to be initialized.

#include <stdbool.h>

#include "event_queue.h"

// Active GPIO state of the latching hazard button
#define HAZARD_BUTTON_ACTIVE_STATE GPIO_STATE_HIGH

bool hazard_tx_process_event(Event *e);
