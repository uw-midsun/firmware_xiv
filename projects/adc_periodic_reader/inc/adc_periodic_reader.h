#pragma once

#include "adc.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "misc.h"

typedef struct {
    EventId event;
}AdcPeriodicReaderStorage;

//Initialize the pins to recieve analog signals
//Initialize pin and settings
StatusCode adc_periodic_reader_init(AdcPeriodicReaderStorage* storage, GpioSettings* settings);

//Enable ADC Channel
StatusCode adc_periodic_reader_set_up_channel(AdcPeriodicReaderStorage* storage);

void analog_reader_register_callback(AdcChannel channel);
