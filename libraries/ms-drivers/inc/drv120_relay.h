#pragma once

// Driver for the DRV120 relay
// Requires GPIO to be initialized

#include <stdbool.h>

#include "gpio.h"
#include "status.h"

// Initialize pin for output to open/close relay
// Pin taken in is the EN pin on the DRV120: high = closed, low = open
// Relay defaults open on initialization
StatusCode drv120_relay_init(GpioAddress *pin);

StatusCode drv120_relay_close(void);

StatusCode drv120_relay_open(void);

// Put whether the relay is closed into |closed|.
StatusCode drv120_relay_get_is_closed(bool *closed);
