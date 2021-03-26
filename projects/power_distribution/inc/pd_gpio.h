#pragma once

// Receive events and set the PCA9539R GPIO pin states as specified.
// Requires the event queue, GPIO, interrupts, soft timers, ADC, I2C, and output to be initialized.

#include "event_queue.h"
#include "output.h"
#include "pca9539r_gpio_expander.h"

typedef enum {
  PD_GPIO_STATE_OFF = 0,
  PD_GPIO_STATE_ON,

  // The following states depend on the value in the event's data field.
  // We map 0 in the event data to OFF and any nonzero value there to ON.
  PD_GPIO_STATE_SAME_AS_DATA,      // use the same state as in the event data
  PD_GPIO_STATE_OPPOSITE_TO_DATA,  // use the opposite state as in the event data

  NUM_PD_GPIO_STATES,
} PdGpioState;

typedef struct {
  Output output;
  PdGpioState state;  // state to set it to
} PdGpioOutputSpec;

typedef struct {
  EventId event_id;           // the event to act upon
  PdGpioOutputSpec *outputs;  // an array of outputs specs to use when received
  uint8_t num_outputs;        // length of preceding array
} PdGpioEventSpec;

typedef struct {
  PdGpioEventSpec *events;  // an array of all event specifications to use
  uint8_t num_events;       // length of preceding array
} PdGpioConfig;

StatusCode power_distribution_gpio_init(PdGpioConfig *config);

StatusCode power_distribution_gpio_process_event(Event *e);
