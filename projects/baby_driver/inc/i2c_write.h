#pragma once

// This module allows data to be written over i2c. It listens for a series of i2c_write
// babydriver CAN messages and stores the data before writing them over i2c.
// Requires dispatcher, interrupts, gpio, the event queue, and CAN to be initialized.

#include "status.h"

StatusCode i2c_write_init(void);
