#include "bts_7200_current_sense.h"
#include "adc.h"
#include "soft_timer.h"

#define BTS_7200_UPDATE_INTERVAL_US 1000 // ok? 

static const SoftTimerId s_timer_id = 0xABCD; // how to set this?

static void prv_measure_current(SoftTimerId timer_id, void *context) {
  Bts7200Storage *storage = context;
  
  gpio_set_state(storage->select_pin, GPIO_STATE_LOW);
  adc_read_raw(storage->sense_channel, &storage->reading_low);
  
  gpio_set_state(storage->select_pin, GPIO_STATE_HIGH);
  adc_read_raw(storage->sense_channel, &storage->reading_high);
  
  soft_timer_start(BTS_7200_UPDATE_INTERVAL_US, &prv_measure_current, storage, &timer_id);
}

void bts_7200_init(Bts7200Settings *settings, Bts7200Storage *storage) {
  storage->select_pin = settings->select_pin;
  
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
  
  // set up a soft timer to update the storage with the measurements
  soft_timer_start(BTS_7200_UPDATE_INTERVAL_US, &prv_measure_current, storage, &s_timer_id);
}
