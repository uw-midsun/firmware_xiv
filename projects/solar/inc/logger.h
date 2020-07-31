#pragma once

// Logs all the data in the data store to the console on every data ready event.
// Requires the event queue and the data store to be initialized.

#include <stdbool.h>

#include "event_queue.h"
#include "solar_boards.h"
#include "status.h"

// Initialize the module, logging data from the given number of MPPTs on the board.
StatusCode logger_init(SolarMpptCount mppt_count);

// Process the event and return whether the event was processed.
bool logger_process_event(Event *e);
