#pragma once

// Driver for the DRV120 relay
// Requires GPIO to be initialized

#include "gpio.h"
#include "status.h"

// Initialize pin for output to open/close relay
// Pin taken in is the EN pin on the DRV120: high = closed, low = open
// Relay defaults closed on initialization
StatusCode drv120_relay_init(GpioAddress *pin);

StatusCode drv120_relay_close(void);

StatusCode drv120_relay_open(void);
