#pragma once

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"

// This module simply watches for the BPS_HEARTBEAT
// to make sure that no fault has occured

// Requires the event_queue and can
// to be initialized
// as it registers a can_rx_handler for BPS_HEARTBEAT
StatusCode bps_watcher_init(void);
