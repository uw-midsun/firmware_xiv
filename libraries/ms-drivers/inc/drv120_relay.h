#pragma once

// Driver for the DRV120 relay
// Requires GPIO to be initialized

#include "gpio.h"
#include "status.h"

StatusCode drv120_relay_init(GpioAddress *pin);

StatusCode drv120_relay_close(void);

StatusCode drv120_relay_open(void);
