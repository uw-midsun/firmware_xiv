#pragma once

// This module allows gpio interrupts to be registered and unregistered. Once the interrupt
// is triggered a CAN message is spontaneously produced.
// Requires dispatcher, interrupts, soft timers, gpio, gpio interrupts, the event queue, and CAN to
// be initialized

#include "status.h"

StatusCode gpio_interrupts_init(void);
