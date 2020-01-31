#include "adc_periodic_reader.h"
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"

#define TIMER_INTERVAL 50u

StatusCode adc_periodic_reader_init(AdcPeriodicReaderStorage *storage, GpioSettings *settings,
                                    size_t storage_size) {
  for (size_t i = 0; i < storage_size; i++) {
    gpio_init_pin(&storage[i].address, settings);
  }
  adc_init(ADC_MODE_SINGLE);

  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_set_up_channel(AdcPeriodicReaderStorage *storage,
                                              size_t storage_size) {
  for (size_t i = 0; i < storage_size; i++) {
    adc_get_channel(storage[i].address, &storage[i].channel);
    adc_set_channel(storage[i].channel, true);
  }
  return STATUS_CODE_OK;
}

StatusCode analog_reader_register_callback(SoftTimerCallback callback,
                                           AdcPeriodicReaderStorage storage) {
  soft_timer_start_millis(TIMER_INTERVAL, callback, &storage, NULL);
  return STATUS_CODE_OK;
}
