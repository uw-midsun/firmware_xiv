#pragma once

// GPIO interface to an arbitrary-bit multiplexer like SN74LV4051AQPWRQ1 or CD74HC4067.
// Requires GPIO to be initialized.

#include "gpio.h"

#define MAX_MUX_BIT_WIDTH 8

// The first bit_width sel_pins must be valid. bit_width must be <= MAX_MUX_BIT_WIDTH.
typedef struct {
  uint8_t bit_width;
  GpioAddress sel_pins[MAX_MUX_BIT_WIDTH];
  GpioAddress mux_output_pin;
  GpioAddress mux_enable_pin;
} MuxAddress;

// Initialize all the pins of the given mux.
// Note: this will initialize the mux_output_pin as input with GPIO_ALTFN_NONE. If you want a
// different configuration, reinitialize it. (Arshan what should I do)
StatusCode mux_init(MuxAddress *address);

// Set the select pins to get the specified pin.
StatusCode mux_set(MuxAddress *address, uint8_t selected);
