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

static AdcPeriodicReaderStorage s_storage[NUM_PERIODIC_READER_IDS];
static uint32_t timer_interval_ms = 0;

void prv_callback(SoftTimerId timer_id, void *context) {
  for (size_t i = 0; i < NUM_PERIODIC_READER_IDS; i++) {
    uint16_t data = 0;
    AdcChannel channel;
    if (s_storage[i].activated) {
      adc_get_channel(s_storage[i].address, &channel);
      adc_read_converted(channel, &data);
      s_storage[i].data = data;
      s_storage[i].callback(s_storage[i].data, i, s_storage[i].context);
    }
  }
  soft_timer_start_millis(timer_interval_ms, prv_callback, NULL, NULL);
}

StatusCode adc_periodic_reader_init(uint32_t reader_interval_ms) {
  timer_interval_ms = reader_interval_ms;
  soft_timer_start_millis(timer_interval_ms, prv_callback, NULL, NULL);
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
  adc_get_channel(s_storage[reader_id].address, &channel);
  adc_set_channel(channel, true);
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
