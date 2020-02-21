#include "adc_periodic_reader.h"
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

#define TIMER_INTERVAL 50

static AdcPeriodicReaderStorage s_storage[NUM_PERIODIC_READER_IDS];

void prv_callback(SoftTimerId timer_id, void *context) {
  for (size_t i = 0; i < NUM_PERIODIC_READER_IDS; i++) {
    if (s_storage[i].activated) {
      s_storage[i].callback(s_storage[i].data, i, s_storage[i].context);
    }
  }
  soft_timer_start_millis(TIMER_INTERVAL, prv_callback, NULL, NULL);
}

StatusCode adc_periodic_reader_init() {
  soft_timer_start_millis(TIMER_INTERVAL, prv_callback, NULL, NULL);

  // Disable all ADCs
  for (size_t i = 0; i < NUM_PERIODIC_READER_IDS; i++) {
    s_storage[i].activated = false;
  }

  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_set_up_reader(PeriodicReaderId reader_id,
                                             AdcPeriodicReaderSettings *adc_settings) {
  GpioSettings gpio_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  if (adc_settings->address.pin >= GPIO_PINS_PER_PORT ||
      adc_settings->address.port >= NUM_GPIO_PORTS || reader_id >= NUM_PERIODIC_READER_IDS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_storage[reader_id].address.pin = adc_settings->address.pin;
  s_storage[reader_id].address.port = adc_settings->address.port;
  s_storage[reader_id].callback = adc_settings->callback;
  s_storage[reader_id].context = adc_settings->context;

  AdcChannel channel;
  status_ok_or_return(gpio_init_pin(&s_storage[reader_id].address, &gpio_settings));
  status_ok_or_return(adc_get_channel(s_storage[reader_id].address, &channel));
  status_ok_or_return(adc_set_channel(channel, true));
  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_start(PeriodicReaderId reader_id) {
  if (reader_id >= NUM_PERIODIC_READER_IDS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_storage[reader_id].activated = true;
  return STATUS_CODE_OK;
}

StatusCode adc_periodic_reader_stop(PeriodicReaderId reader_id) {
  if (reader_id >= NUM_PERIODIC_READER_IDS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_storage[reader_id].activated = false;
  return STATUS_CODE_OK;
}
