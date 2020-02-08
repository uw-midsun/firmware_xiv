#pragma once

// GPIO analog input interface through SN74LV4051AQPWRQ1 3-bit multiplexer.
// Requires GPIO to be initialized.

#include "gpio.h"

#define SN74_MUX_BIT_WIDTH 3

typedef struct {
  GpioAddress sel_pins[SN74_MUX_BIT_WIDTH];
  GpioAddress mux_output_pin;
} Sn74MuxAddress;

// Initialize all the pins of the given mux.
StatusCode sn74_mux_init_mux(Sn74MuxAddress *address);

// Set the select pins to get the specified pin.
StatusCode sn74_mux_set(Sn74MuxAddress *address, uint8_t selected);
