#pragma once

// Receives button presses on GPIO, and raises events.
// Requires interrupts and gpio to be initialized.

// CentreConsoleButtonPressEvents are raised. The event data is the state of the button pin at the
// time of the interrupt; this is relevant for latching buttons. The interrupt is triggered on the
// rising edge for non-latching buttons and on both edges for latching buttons.

#include "gpio.h"
#include "status.h"

typedef enum {
  CENTRE_CONSOLE_BUTTON_POWER = 0,
  CENTRE_CONSOLE_BUTTON_PARKING,
  CENTRE_CONSOLE_BUTTON_HAZARDS,
  CENTRE_CONSOLE_BUTTON_DRIVE,
  CENTRE_CONSOLE_BUTTON_NEUTRAL,
  CENTRE_CONSOLE_BUTTON_REVERSE,
  NUM_CENTRE_CONSOLE_BUTTONS
} CentreConsoleButton;

StatusCode button_press_init(void);

GpioAddress *test_provide_button_addresses(void);
