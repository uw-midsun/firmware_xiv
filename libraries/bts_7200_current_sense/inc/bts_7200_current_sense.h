#pragma once

// Reads current values from 2 ADC pins on the BTS 7200 switched between with a selection pin.
// Requires GPIO, soft timers, and ADC to be initialized in ADC_MODE_SINGLE.

#include "gpio.h"
#include "adc.h"

typedef struct {
  GpioAddress *select_pin;
  GpioAddress *sense_pin;
} Bts7200Settings;

typedef struct {
  uint16_t reading_low;
  uint16_t reading_high;
  GpioAddress *select_pin;
  AdcChannel sense_channel;
  SoftTimerId timer_id;
} Bts7200Storage;

// Set up a soft timer which periodically updates |storage| with the latest measurements.
// |storage| is immediately updated with measurements.
void bts_7200_init(Bts7200Settings *settings, Bts7200Storage *storage);

void bts_7200_cancel(Bts7200Storage *storage);
