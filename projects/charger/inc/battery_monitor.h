#pragma once

// requires event_queue and CAN to be initialized
// listens for battery CAN messages and starts the stop_fsm if charged

#include "status.h"

#define CHARGER_BATTERY_THRESHOLD 1350  // decivolts, = 135.0 volts

StatusCode battery_monitor_init();
