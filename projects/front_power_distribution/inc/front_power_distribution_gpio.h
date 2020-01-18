#pragma once

// Receive GPIO events and set the corresponding GPIO pin states.
// Requires GPIO to be initialized.

#include "event_queue.h"
#include "gpio.h"

void front_power_distribution_gpio_init(void);

// Call in main event-processing loop.
void front_power_distribution_gpio_process_event(Event *e);
