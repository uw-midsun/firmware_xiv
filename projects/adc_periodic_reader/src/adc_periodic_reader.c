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

StatusCode adc_periodic_reader_init(AdcPeriodicReaderStorage *storage, GpioSettings *settings) {
  gpio_init();
  gpio_it_init();
  interrupt_init();

  gpio_init_pin(&storage->address, settings);
  adc_init(ADC_MODE_SINGLE);

  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_set_up_channel(AdcPeriodicReaderStorage *storage) {
  adc_set_channel(storage->channel, true);
  return STATUS_CODE_OK;
}

StatusCode analog_reader_register_callback(AdcCallback callback,
                                           AdcPeriodicReaderStorage *storage) {
  adc_register_callback(storage->channel, callback, storage->data);
  return STATUS_CODE_OK;
}
