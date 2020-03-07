#pragma once

// Receive GPIO events and set the corresponding GPIO pin states.
// Requires GPIO, I2C, and PCA9539R to be initialized.

// NOT YET GENERIC

#include "event_queue.h"
#include "pca9539r_gpio_expander.h"

// Outputs represent the distinct pins that can be set. They do not form an exact 1-to-1 mapping to
// events; namely, the POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD event corresponds to both
// the POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT and POWER_DISTRIBUTION_SIGNAL_RIGHT outputs.
typedef enum {
  POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY = 0,
  POWER_DISTRIBUTION_OUTPUT_STEERING,
  POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE,
  POWER_DISTRIBUTION_OUTPUT_DRL,
  POWER_DISTRIBUTION_OUTPUT_PEDAL,
  POWER_DISTRIBUTION_OUTPUT_HORN,
  POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT,
  POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT,
  NUM_POWER_DISTRIBUTION_GPIO_OUTPUTS,
} PowerDistributionGpioOutput;

void power_distribution_gpio_init(void);

StatusCode power_distribution_gpio_process_event(Event *e);

Pca9539rGpioAddress *power_distribution_gpio_test_provide_gpio_addresses(void);

PowerDistributionGpioOutput *power_distribution_gpio_test_provide_events_to_outputs(void);
