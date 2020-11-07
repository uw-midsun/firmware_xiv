#pragma once

// Receives button presses on GPIO, and raises events.
// Requires interrupts and gpio to be initialized.

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
