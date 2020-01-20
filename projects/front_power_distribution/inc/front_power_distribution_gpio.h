#pragma once

// Receive GPIO events and set the corresponding GPIO pin states.
// Requires GPIO to be initialized.

#include "event_queue.h"
#include "gpio.h"

void front_power_distribution_gpio_init(void);

StatusCode front_power_distribution_gpio_process_event(Event *e);

GpioAddress *front_power_distribution_gpio_test_provide_gpio_addresses(void);
