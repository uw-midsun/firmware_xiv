#pragma once

// Retrieves pedal states from pedal_monitor.c and transmits CAN messages
// to control brake lights

// Requires pedal_monitor, CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "status.h"

static void brake_light_control_update(PedalState current_state, PedalState new_state);
