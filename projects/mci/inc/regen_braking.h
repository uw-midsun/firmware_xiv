#pragma once

#include <stdbool.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "status.h"

typedef enum {REGEN_DISABLED = 0, REGEN_ENABLED = 1} RegenBrakingState;

StatusCode regen_braking_init(void);

RegenBrakingState get_regen_braking_state(void);
