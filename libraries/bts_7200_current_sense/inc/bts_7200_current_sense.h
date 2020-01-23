#pragma once

// Reads current values from 2 ADC pins on the BTS 7200 switched between with a selection pin.
// Requires GPIO, soft timers, and ADC to be initialized in ADC_MODE_SINGLE.

#include "gpio.h"
#include "adc.h"

typedef struct {
  GpioAddress *select_pin;
  GpioAddress *sense_pin;
  uint32_t interval_us;
} Bts7200Settings;

struct Bts7200Storage;

// Set up a soft timer which periodically updates |storage| with the latest measurements.
// |bts_7200_get_measurement_high| and |bts_7200_get_measurement_low| can immediately be called.
void bts_7200_init(Bts7200Settings *settings, Bts7200Storage *storage);

// Get the measurement when the select pin is high.
uint16_t bts_7200_get_measurement_high(Bts7200Storage *storage);

// Get the measurement when the select pin is low.
uint16_t bts_7200_get_measurement_low(Bts7200Storage *storage);

void bts_7200_cancel(Bts7200Storage *storage);
