#pragma once

// This module sets a specific gpio pin to a desired state
// Requires dispatcher, gpio, and CAN to be initialized.

#include "status.h"

// initialize the module
StatusCode gpio_set_init(void);
