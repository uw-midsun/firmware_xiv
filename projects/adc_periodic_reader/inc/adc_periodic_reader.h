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

typedef void (*AdcPeriodicReaderCallback)(uint16 data, AdcPeriodicReaderId id,
                                          void *context);

typedef enum {
  PERIODIC_READER_ID_0 = 0,
  PERIODIC_READER_ID_1,
  PERIODIC_READER_ID_2,
  PERIODIC_READER_ID_3,
  PERIODIC_READER_ID_4,
  NUM_PERIODIC_READER_IDS
} PeriodicReaderId;

// Do I define the array in here?
// What would I put for the address and callback?
// Doesen't the user define it?

typedef struct {
  GpioAddress address;
  AdcPeriodicReaderCallback callback;
} AdcPeriodicReaderSettings;

 static AdcPeriodicReaderSettings s_storage[NUM_PERIODIC_READER_IDS];
 
// Sets up reader and disables all ADCs
StatusCode adc_periodic_reader_init();

// Intialize pins and enable ADC reader
StatusCode adc_periodic_reader_set_up_reader();

// Enable soft - timer to periodically read voltages
StatusCode adc_periodic_reader_start(AdcPeriodicReaderSetting *settings);

// Disable soft - timer
StatusCode adc_periodic_reader_stop(AdcPeriodicReaderSetting *settings);
