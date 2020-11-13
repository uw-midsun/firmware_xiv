#pragma once

// Driver for the DRV120 relay
// Requires GPIO and Gpio_it to be initialized

#include <stdbool.h>
#include <stdlib.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "status.h"

typedef void (*Drv120ErrorCallback)(void * context);

typedef struct Drv120RelaySettings {
  GpioAddress *enable_pin;        // Pin initialized as output for relay
  GpioAddress *status_pin;     // Pin set to trigger interrupt on error
  Drv120ErrorCallback error_handler; 
  void *context;           // Context for error handler
} Drv120RelaySettings;

// Initialize pin for output to open/close relay
// Pin taken in is the EN pin on the DRV120: high = closed, low = open
// Relay defaults open on initialization
// If status pin set as NULL, error interrupt not initialized
StatusCode drv120_relay_init(Drv120RelaySettings *settings);

StatusCode drv120_relay_close(void);

StatusCode drv120_relay_open(void);

// Put whether the relay is closed into |closed|.
StatusCode drv120_relay_get_is_closed(bool *closed);
