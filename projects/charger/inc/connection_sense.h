#pragma once

// periodically polls the connection pin and raises events upon state change
// requires gpio, ADC, soft timers, CAN, and event queue to be initialized

#include "status.h"

#define CONNECTION_SENSE_POLL_PERIOD_MS 500

#define CONNECTION_SENSE_GPIO_ADDR \
  { GPIO_PORT_A, 7 }

typedef enum {
  CHARGER_STATE_UNPLUGGED = 0,
  CHARGER_STATE_PLUGGED_PRESSED,
  CHARGER_STATE_PLUGGED_RELEASED,
  NUM_CHARGER_CONNECTION_STATES
} ConnectionState;

StatusCode connection_sense_init();
