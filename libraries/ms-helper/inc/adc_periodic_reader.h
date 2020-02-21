#pragma once

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
/*
Requires
-Soft Timers
-Interrupts
-GPIO
-ADC (ADC Mode Single)
*/
typedef enum {
  PERIODIC_READER_ID_0 = 0,
  PERIODIC_READER_ID_1,
  PERIODIC_READER_ID_2,
  PERIODIC_READER_ID_3,
  PERIODIC_READER_ID_4,
  NUM_PERIODIC_READER_IDS
} PeriodicReaderId;

typedef void (*AdcPeriodicReaderCallback)(uint16_t data, PeriodicReaderId id, void *context);

typedef struct {
  GpioAddress address;
  AdcPeriodicReaderCallback callback;
  void *context;
} AdcPeriodicReaderSettings;

typedef struct {
  GpioAddress address;
  AdcPeriodicReaderCallback callback;
  uint16_t data;
  bool activated;
  void *context;
} AdcPeriodicReaderStorage;

// Sets soft-timer and disables all ADCs
StatusCode adc_periodic_reader_init();

// Intialize pins and enable ADC reader
StatusCode adc_periodic_reader_set_up_reader(PeriodicReaderId reader_id,
                                             AdcPeriodicReaderSettings *adc_settings);
// Enables a specific ADC
StatusCode adc_periodic_reader_start(PeriodicReaderId reader_id);

StatusCode adc_periodic_reader_stop(PeriodicReaderId reader_id);
