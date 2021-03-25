#pragma once

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"

// This module simply watches for the BPS_HEARTBEAT
// to make sure that no fault has occured
StatusCode bps_watcher_init(void);
