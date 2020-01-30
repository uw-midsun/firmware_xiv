#pragma once

// Receives button presses on GPIO, and raises events.
// Requires interrupts and gpio to be initialized.

#include "status.h"
#include "gpio.h"

typedef enum {
  CENTRE_CONSOLE_BUTTON_DRIVE = 0,
  CENTRE_CONSOLE_BUTTON_REVERSE,
  CENTRE_CONSOLE_BUTTON_POWER,
  CENTRE_CONSOLE_BUTTON_NEUTRAL,
  CENTRE_CONSOLE_BUTTON_HAZARD,
  CENTRE_CONSOLE_BUTTON_EMERGENCY_STOP,
  NUM_CENTRE_CONSOLE_BUTTONS
} CentreConsoleButton;

StatusCode button_press_init(void);

GpioAddress *test_provide_button_addresses(void);
