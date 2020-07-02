#pragma once

// listens for battery CAN messages and starts the stop_fsm if charged
// requires event_queue and CAN to be initialized

#include "status.h"

StatusCode battery_monitor_init();
