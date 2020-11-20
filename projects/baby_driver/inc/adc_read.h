#pragma once

// This module provides the function adc_read_init() which can be used to listen for
// adc_read_command CAN messages from the babydriver. Once the message is received, the
// ADC reading from the specificed port and pin is sent back to Python using a CAN message.
// Requires dispatcher, interrupts, gpio, the event queue, and CAN to be initialized.

#include "status.h"

// Initialize module
StatusCode adc_read_init(void);
