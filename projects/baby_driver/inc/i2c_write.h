#pragma once

// This module allows data to be written over i2c. It listens for a series of i2c_write
// babydriver CAN messages and stores the data before writing them over i2c.
// Requires dispatcher, interrupts, soft timers, gpio, the event queue, and CAN to be initialized.

#include "i2c.h"
#include "status.h"

typedef struct I2CWriteCommand {
  I2CPort port;
  I2CAddress address;
  uint8_t tx_len;
  uint8_t is_reg;
  uint8_t reg;
} I2CWriteCommand;

#define I2C_WRITE_DEFAULT_TIMEOUT_MS 750

// Timeout period can be adjusted - mostly for testing
StatusCode i2c_write_init(uint32_t timeout_ms);
