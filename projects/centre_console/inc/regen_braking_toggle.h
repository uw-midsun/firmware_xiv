#pragma once

// A module to toggle the regen braking state, and reverts the state if the
// ACK on the REGEN_BRAKING message fails
// Requires the event queue, interrupts, soft timers, gpio, and CAN to be initialized.

#include <stdbool.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_pack.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "status.h"

// Initializes regen braking by registering a handler for the toggle request
// and sets the regen braking state to false by default
StatusCode regen_braking_toggle_init(void);

// Returns the state of regen braking
bool get_regen_braking_state(void);

// Function used to manually set the regen braking state, if attempting to set the
// same state, the request is silently ignored, and no messages are sent
StatusCode set_regen_braking_state(bool state);

// Once power main sequence completes, regen braking is enabled
bool regen_braking_process_event(Event *e);
