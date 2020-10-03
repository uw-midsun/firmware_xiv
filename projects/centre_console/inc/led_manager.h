#pragma once

// Toggles the button LEDs upon receiving the appropraite events.
// Requires gpio, the event queue, and I2C (on I2C_PORT_2) to be initialized.

#include <stdbool.h>

#include "event_queue.h"
#include "status.h"

// TODO(SOFT-296): make this accurate to reality whenever CC rev 2 gets uploaded to A365
typedef enum {
  CENTRE_CONSOLE_LED_BPS = 0,
  CENTRE_CONSOLE_LED_POWER,
  CENTRE_CONSOLE_LED_REVERSE,
  CENTRE_CONSOLE_LED_NEUTRAL,
  CENTRE_CONSOLE_LED_DRIVE,
  CENTRE_CONSOLE_LED_DRL,       // this doesn't exist in rev 2?
  CENTRE_CONSOLE_LED_LOW_BEAM,  // neither does this?
  CENTRE_CONSOLE_LED_HAZARDS,
  // I think there's a parking one in rev 2?
  NUM_CENTRE_CONSOLE_LEDS,
} CentreConsoleLed;

// Initialize the module.
StatusCode led_manager_init(void);

// Process the given event and return whether the event was processed (i.e. an LED was toggled).
bool led_manager_process_event(Event *e);
