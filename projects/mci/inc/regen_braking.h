#pragma once

#include <stdbool.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "status.h"

StatusCode regen_braking_init(void);

uint8_t get_regen_braking_state(void);
