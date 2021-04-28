#pragma once

// This module allows for data to be read from an IC over I2C at a given I2C port and address,
// can also perform a register read instead of a regular read if specified.
// Requires dispatcher, interrupts, soft timers, gpio, the event queue, and CAN to be initialized.

#include "i2c.h"
#include "status.h"

typedef struct I2CReadCommand {
  I2CPort port;
  I2CAddress address;
  uint8_t rx_len;
  uint8_t is_reg;
  uint8_t reg;
} I2CReadCommand;

#define I2C_WRITE_DEFAULT_TIMEOUT_MS 750

// Timeout period can be made shorter for testing
StatusCode i2c_read_init(uint32_t timeout_ms, uint32_t timeout_soft_timer);
