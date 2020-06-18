#pragma once

// requires event_queue and CAN to be initialized
// listens for battery CAN messages and starts the stop_fsm if charged

#include "status.h"

StatusCode battery_monitor_init();
