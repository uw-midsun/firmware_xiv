#pragma once

// GPIO interface to an arbitrary-bit multiplexer or demultiplexer.
// Requires GPIO to be initialized.

#include "gpio.h"

#define MAX_MUX_BIT_WIDTH 8

// The first bit_width sel_pins must be valid. bit_width must be <= MAX_MUX_BIT_WIDTH.
typedef struct {
  uint8_t bit_width;
  GpioAddress sel_pins[MAX_MUX_BIT_WIDTH];
} MuxAddress;

// Initialize all the select pins of the given mux.
StatusCode mux_init(MuxAddress *address);

// Set the select pins to get the specified pin.
StatusCode mux_set(MuxAddress *address, uint8_t selected);
