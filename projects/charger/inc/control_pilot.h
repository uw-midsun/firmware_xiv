#pragma once

// This module wraps PWM to facilitate getting the current from a PWM reading
// Used by the begin_sequence module.
// requires gpio to be initialized

#include <stdint.h>

#include "status.h"

#define CONTROL_PILOT_PWM_GPIO_ADDR \
  { GPIO_PORT_A, 6 }

uint16_t control_pilot_get_current();

StatusCode control_pilot_init();
