#pragma once

// This module provides the function spi_exchange_init()
// for spi_exchange CAN messages from the babydriver. Once the message is received,
// the spi_init is run with the inputs and the spi_exchange function is run

#include <stdbool.h>
#include <stdint.h>

#include "babydriver_msg_defs.h"
#include "status.h"

// Initialize the module
StatusCode spi_exchange_init(void);
