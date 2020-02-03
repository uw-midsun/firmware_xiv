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

void prv_callback(SoftTimerId timer_id, void *context) {
  for (size_t i = 0; i < NUM_PERIODIC_READER_IDS; i++) {
    if (s_settings[i].activated) {
      s_settings[i].callback(s_storage[i].data, i, NULL);
    }
  }
}

StatusCode adc_periodic_reader_init() {
  gpio_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);
  soft_timer_start_millis(TIMER_INTERVAL, prv_callback, NULL, NULL);

  // Disable all ADCs
  for (size_t i = 0; i < NUM_PERIODIC_READER_IDS; i++) {
    s_settings[i].activated = false;
  }

  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_set_up_reader(PeriodicReaderId reader_id,
                                             GpioSettings *gpio_settings) {
  AdcChannel channel;
  gpio_init_pin(&s_settings[reader_id].address, gpio_settings);
  adc_get_channel(s_settings[reader_id].address, &channel);
  adc_set_channel(channel, true);
  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_start(PeriodicReaderId reader_id) {
  s_settings[reader_id].activated = true;
  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_stop(PeriodicReaderId reader_id) {
  s_settings[reader_id].activated = false;
  return STATUS_CODE_OK;
}
