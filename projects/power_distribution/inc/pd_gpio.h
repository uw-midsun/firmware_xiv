#pragma once

// Receive events and set the PCA9539R GPIO pin states as specified.
// Requires the event queue, GPIO, interrupts, soft timers, ADC, I2C, and output to be initialized.

#include "event_queue.h"
#include "output.h"
#include "pca9539r_gpio_expander.h"

typedef enum {
  POWER_DISTRIBUTION_GPIO_STATE_OFF = 0,
  POWER_DISTRIBUTION_GPIO_STATE_ON,

  // The following states depend on the value in the event's data field.
  // We map 0 in the event data to OFF and any nonzero value there to ON.
  POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA,      // use the same state as in the event data
  POWER_DISTRIBUTION_GPIO_STATE_OPPOSITE_TO_DATA,  // use the opposite state as in the event data

  NUM_POWER_DISTRIBUTION_GPIO_STATES,
} PowerDistributionGpioState;

typedef struct {
  Output output;
  PowerDistributionGpioState state;  // state to set it to
} PowerDistributionGpioOutputSpec;

typedef struct {
  EventId event_id;                          // the event to act upon
  PowerDistributionGpioOutputSpec *outputs;  // an array of outputs specs to use when received
  uint8_t num_outputs;                       // length of preceding array
} PowerDistributionGpioEventSpec;

typedef struct {
  PowerDistributionGpioEventSpec *events;  // an array of all event specifications to use
  uint8_t num_events;                      // length of preceding array

  // An array of all addresses used, together with their default states.
  // POWER_DISTRIBUTION_GPIO_STATE_SAME and POWER_DISTRIBUTION_GPIO_STATE_OPPOSITE cannot be
  // used here - only POWER_DISTRIBUTION_GPIO_STATE_ON and POWER_DISTRIBUTION_GPIO_STATE_OFF.
  PowerDistributionGpioOutputSpec *all_addresses_and_default_states;
  uint8_t num_addresses;
} PowerDistributionGpioConfig;

StatusCode power_distribution_gpio_init(PowerDistributionGpioConfig *config);

StatusCode power_distribution_gpio_process_event(Event *e);
