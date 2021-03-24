#pragma once

// Handles responding during main power on sequence with status of power select aux/dcdc

#include "can.h"
#include "power_select.h"
#include "power_select_events.h"
#include "exported_enums.h"
#include "can_pack.h"
#include "can_unpack.h"
#include "log.h"

// Initialize CAN to respond to POWER_ON_MAIN_SEQUENCE messages
StatusCode power_select_can_init(void);