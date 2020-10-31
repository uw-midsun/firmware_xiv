#pragma once

// This module provides the function gpio_get_init(), which can be used to listen
// for gpio_get CAN messages from the babydriver. Once the message is received,
// the state of the requested GPIO pin is obtained and sent back to the baby driver using
// CAN_TRANSMIT_BABYDRIVER

#include <stdbool.h>
#include <stdint.h>

#include "babydriver_msg_defs.h"
#include "status.h"

StatusCode gpio_get_init(void);
