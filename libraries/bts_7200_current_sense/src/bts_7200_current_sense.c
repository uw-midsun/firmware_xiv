#include "bts_7200_current_sense.h"

static void prv_measure_current(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  
  gpio_set_state(storage->select_pin, GPIO_STATE_LOW);
  adc_read_raw(storage->sense_channel, &storage->reading_low);
  
  gpio_set_state(storage->select_pin, GPIO_STATE_HIGH);
  adc_read_raw(storage->sense_channel, &storage->reading_high);
  
  storage->callback();
  
  soft_timer_start(storage->interval_us, &prv_measure_current, storage, &storage->timer_id);
}

void bts_7200_init(Bts7200Settings *settings, Bts7200Storage *storage) {
  storage->select_pin = settings->select_pin;
  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;
  
  // initialize the select pin
  GpioSettings select_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  gpio_init_pin(settings->select_pin, &select_settings);
  
  // initialize the sense pin
  GpioSettings sense_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  gpio_init_pin(settings->sense_pin, &sense_settings);
  
  // initialize the sense pin as ADC
  storage->sense_channel = NUM_ADC_CHANNELS;
  adc_get_channel(*settings->sense_pin, &storage->sense_channel);
  adc_set_channel(storage->sense_channel, true);
  
  // |prv_measure_current| will set up a soft timer to call itself repeatedly
  // this way we have a valid measurement immediately and there's no period with invalid measurements
  prv_measure_current(SOFT_TIMER_INVALID_TIMER, storage);
}

uint16_t bts_7200_get_measurement_high(Bts7200Storage *storage) {
  return storage->reading_high;
}

uint16_t bts_7200_get_measurement_low(Bts7200Storage *storage) {
  return storage->reading_low;
}

bool bts_7200_cancel(Bts7200Storage *storage) {
  return soft_timer_cancel(storage->timer_id);
}
