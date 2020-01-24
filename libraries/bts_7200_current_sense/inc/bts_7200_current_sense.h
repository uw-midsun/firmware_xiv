#pragma once

// Reads current values from 2 ADC pins on the BTS 7200 switched between with a selection pin.
// Requires GPIO, interrupts, soft timers, and ADC to be initialized in ADC_MODE_SINGLE.

#include "adc.h"
#include "gpio.h"
#include "soft_timer.h"

typedef void (*Bts7200MeasurementsReadyCallback)(void);

typedef struct {
  GpioAddress *select_pin;
  GpioAddress *sense_pin;
  uint32_t interval_us;
  Bts7200MeasurementsReadyCallback callback;
} Bts7200Settings;

typedef struct Bts7200Storage {
  uint16_t reading_low;
  uint16_t reading_high;
  GpioAddress *select_pin;
  uint32_t interval_us;
  AdcChannel sense_channel;
  SoftTimerId timer_id;
  Bts7200MeasurementsReadyCallback callback;
} Bts7200Storage;

// Set up a soft timer which periodically updates |storage| with the latest measurements.
// |bts_7200_get_measurement_high| and |bts_7200_get_measurement_low| can immediately be called.
void bts_7200_init(Bts7200Settings *settings, Bts7200Storage *storage);

// Get the measurement when the select pin is high.
uint16_t bts_7200_get_measurement_high(Bts7200Storage *storage);

// Get the measurement when the select pin is low.
uint16_t bts_7200_get_measurement_low(Bts7200Storage *storage);

// Cancel the timer associated with the storage and return whether it was successful.
bool bts_7200_cancel(Bts7200Storage *storage);
