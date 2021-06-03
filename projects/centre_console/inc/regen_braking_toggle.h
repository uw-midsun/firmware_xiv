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

StatusCode regen_braking_init();

bool get_regen_braking_state();

StatusCode set_regen_braking(bool state);

bool regen_braking_process_event(Event *e);
