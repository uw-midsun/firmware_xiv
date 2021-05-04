#pragma once

// Retrieves pedal states from pedal_monitor.c and transmits CAN messages
// to control brake lights

// Requires pedal_monitor, CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include <stdbool.h>

#include "status.h"

bool brake_light_control_process_event(Event *e);
