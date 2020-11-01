#pragma once

// This module allows a specific gpio pin to be set to a desired state when a
// babydriver CAN message with a specific ID is received, using the dispatcher.
// Requires dispatcher, and gpio to be initialized.

#include "status.h"

// initialize the module
StatusCode gpio_set_init(void);
