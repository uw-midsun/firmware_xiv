#pragma once

// Receive events and set the PCA9539R GPIO pin states as specified.
// Requires GPIO, I2C, and PCA9539R to be initialized.

#include "event_queue.h"
#include "pca9539r_gpio_expander.h"

typedef enum {
  POWER_DISTRIBUTION_GPIO_STATE_LOW = 0,
  POWER_DISTRIBUTION_GPIO_STATE_HIGH,

  // The following states depend on the value in the event's data field.
  // We map 0 in the event data to LOW and any nonzero value there to HIGH.
  POWER_DISTRIBUTION_GPIO_STATE_SAME,      // use the same state as in the event data
  POWER_DISTRIBUTION_GPIO_STATE_OPPOSITE,  // use the opposite state as in the event data

  NUM_POWER_DISTRIBUTION_GPIO_STATES,
} PowerDistributionGpioState;

typedef struct {
  Pca9539rGpioAddress address;       // GPIO address to turn on/off
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

StatusCode power_distribution_gpio_init(PowerDistributionGpioConfig config);

StatusCode power_distribution_gpio_process_event(Event *e);
