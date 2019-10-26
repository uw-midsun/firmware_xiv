#pragma once
// Wrapper for the controller board's debug LEDs
// GPIO must be initialized.
//
// Assumes the LEDs are as follows:
// Blue: PB5
// Blue: PB4
// Green: PB3
// Red: PA15
#include <stdbool.h>
#include "gpio.h"
#include "status.h"

typedef enum {
  DEBUG_LED_BLUE_A,
  DEBUG_LED_BLUE_B,
  DEBUG_LED_GREEN,
  DEBUG_LED_RED,
  NUM_DEBUG_LEDS,
} DebugLed;

// Initializes the specified debug LED.
// We configure the LED only if explicitly initialized
// in case we're sharing the pin with something else.
StatusCode debug_led_init(DebugLed led);

// Sets the specified debug LED's state. It must have been initialized first.
StatusCode debug_led_set_state(DebugLed led, bool on);

// Toggles the specified debug LED's state. It must have been initialized first.
StatusCode debug_led_toggle_state(DebugLed led);
