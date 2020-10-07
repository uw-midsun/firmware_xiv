#pragma once

// Toggles the button LEDs upon receiving the appropriate events.
// Requires gpio, the event queue, and I2C (on I2C_PORT_2) to be initialized.

#include <stdbool.h>

#include "event_queue.h"
#include "status.h"

typedef enum {
  CENTRE_CONSOLE_LED_BPS = 0,
  CENTRE_CONSOLE_LED_POWER,
  CENTRE_CONSOLE_LED_DRIVE,
  CENTRE_CONSOLE_LED_REVERSE,
  CENTRE_CONSOLE_LED_NEUTRAL,
  CENTRE_CONSOLE_LED_PARKING,
  CENTRE_CONSOLE_LED_HAZARDS,
  CENTRE_CONSOLE_LED_SPARE,
  NUM_CENTRE_CONSOLE_LEDS
} CentreConsoleLed;

// Initialize the module.
StatusCode led_manager_init(void);

// Process the given event and return whether the event was processed (i.e. an LED was toggled).
bool led_manager_process_event(Event *e);
