#pragma once

// This module allows for data to be read from an IC over I2C at a given I2C port and address,
// can also perform a register read instead of a regular read if specified.
// Requires dispatcher, interrupts, soft timers, gpio, the event queue, and CAN to be initialized.

#include "i2c.h"
#include "status.h"

#define I2C_READ_DEFAULT_TX_DELAY_MS 5

// Timeout period can be made shorter for testing
StatusCode i2c_read_init(uint32_t tx_delay_ms);
