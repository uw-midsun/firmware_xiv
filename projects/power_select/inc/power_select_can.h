#pragma once
// Handles responding during main + aux power on sequences with status of power select aux/dcdc
// Requires interrupts, can, soft timers, and event queue to be initialized.

#include "can.h"
#include "can_pack.h"
#include "can_unpack.h"
#include "controller_board_pins.h"
#include "exported_enums.h"
#include "log.h"

#include "power_select.h"
#include "power_select_events.h"

// Initialize CAN to respond to POWER_ON_MAIN_SEQUENCE and POWER_ON_AUX_SEQUENCE messages
StatusCode power_select_can_init(void);
