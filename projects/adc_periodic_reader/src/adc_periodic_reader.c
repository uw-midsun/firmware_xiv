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

#define TIMER_INTERVAL 50

StatusCode adc_periodic_reader_init() {
  gpio_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);
  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_set_up_reader(AdcPeriodicReaderSetting *settings, GpioSettings* gpio_settings) {
  AdcChannel channel;
  gpio_init_pin(&settings->address,gpio_settings)
  adc_get_channel(settings->address,&channel);
  adc_set_channel(channel,true);
  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_start(AdcPeriodicReaderSetting *settings) {
  //For this, create a soft timer callback that take the settings
  //and call the callback that the user gave us
  soft_timer_start_millis(TIMER_INTERVAL, settings->callback, settings, NULL);
}
StatusCode adc_periodic_reader_stop(AdcPeriodicReaderSetting *settings) {
  //What should I make the TimerID?
  //I need TimerId to disable the soft - timer
}