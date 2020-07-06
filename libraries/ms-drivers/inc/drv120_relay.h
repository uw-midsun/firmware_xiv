#pragma once

// Driver for the DRV120 relay
// Requires GPIO and status to be initialized

#include "gpio.h"
#include "status.h"

// Storage for GPIO pin to enable relay
GpioAddress relay_pin;

StatusCode drv120_relay_init(GpioAddress *pin);

StatusCode drv120_relay_close(void);

StatusCode drv120_relay_open(void);
