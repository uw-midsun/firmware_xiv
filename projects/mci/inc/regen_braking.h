#pragma once

// A module to set the regen braking state in MCI
// upon receiving a CAN_TRANSMIT_REGEN_BRAKING can
// message. The regen braking state is used in
// mci_output.c which determines whether we are
// regen braking.
// Requires CAN to be initialized.

#include <stdbool.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "status.h"

StatusCode regen_braking_init(void);

bool get_regen_braking_state(void);
