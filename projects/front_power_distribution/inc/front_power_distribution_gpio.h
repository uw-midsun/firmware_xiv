#pragma once

// Receive GPIO events and set the corresponding GPIO pin states.
// Requires GPIO to be initialized.

#include "event_queue.h"
#include "gpio.h"

// Outputs represent the distinct pins that can be set. They do not form an exact 1-to-1 mapping to
// events; namely, the FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD event corresponds to both
// the FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT and FRONT_POWER_DISTRIBUTION_SIGNAL_RIGHT
// outputs.
typedef enum {
  FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY = 0,
  FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING,
  FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE,
  FRONT_POWER_DISTRIBUTION_OUTPUT_HEADLIGHTS,
  FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_LEFT,
  FRONT_POWER_DISTRIBUTION_OUTPUT_SIGNAL_RIGHT,
  NUM_FRONT_POWER_DISTRIBUTION_GPIO_OUTPUTS,
} FrontPowerDistributionGpioOutput;

void front_power_distribution_gpio_init(void);

StatusCode front_power_distribution_gpio_process_event(Event *e);

GpioAddress *front_power_distribution_gpio_test_provide_gpio_addresses(void);

FrontPowerDistributionGpioOutput *front_power_distribution_gpio_test_provide_events_to_outputs(
    void);
