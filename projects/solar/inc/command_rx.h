#pragma once

// RXs commands to solar over CAN and handles them.
// Requires GPIO, interrupts, soft timers, the event queue, CAN, and relay_fsm to be initialized.

// We mirror the state of the battery relay, so we handle SYSTEM_CAN_MESSAGE_SET_RELAY_STATES
// directed towards EE_RELAY_ID_BATTERY.

#include "status.h"

StatusCode command_rx_init(void);
