#pragma once

// Abstraction layer over the adc_read command

#include <stdbool.h>
#include <stdint.h>

#include "babydriver_msg_defs.h"
#include "status.h"
// #include "dispatcher.h"

// Initialize module
StatusCode adc_read_init(void);

// StatusCode adc_read_callback(void *context);
