#pragma once

#include "adc.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"

typedef struct {
  GpioAddress address;
  AdcChannel channel;
  uint16_t data;
} AdcPeriodicReaderStorage;

// Initialize GPIO pin and settings
StatusCode adc_periodic_reader_init(AdcPeriodicReaderStorage *storage, GpioSettings *settings);

// Enable ADC Channel
StatusCode adc_periodic_reader_set_up_channel(AdcPeriodicReaderStorage *storage);

// Accepts a callback,repeatedly calls that callback with new data as it gets new data
StatusCode analog_reader_register_callback(AdcCallback callback, AdcChannel channel);
