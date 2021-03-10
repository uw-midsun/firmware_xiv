#pragma once

// This module provides the function spi_exchange_init()
// for spi_exchange CAN messages from the babydriver. Once the message is received,
// the spi_init is run with the inputs and the spi_exchange function is run
// Requires dispatcher, interrupts, soft timers, gpio, the event queue, and CAN to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "babydriver_msg_defs.h"
#include "status.h"

// timeout time for watchdog
#define DEFAULT_SPI_EXCHANGE_TIMEOUT_MS (2000)
// delay between each CAN message sent by spi_exchange
#define DEFAULT_SPI_EXCHANGE_TX_DELAY (2000)

// Initialize the module
StatusCode spi_exchange_init(uint32_t spi_exchange_timeout, uint32_t tx_delay);
