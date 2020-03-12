#include "bts_7040_7008_current_sense.h"
#include "adc.h"

StatusCode bts_7040_init(Bts7040Storage *storage, Bts7040Settings *settings) {
  storage->sense_pin = settings->sense_pin;

  // initialize the sense pin as ADC
  GpioSettings sense_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  status_ok_or_return(gpio_init_pin(storage->sense_pin, &sense_settings));

  AdcChannel sense_channel = NUM_ADC_CHANNELS;
  adc_get_channel(*storage->sense_pin, &sense_channel);
  adc_set_channel(sense_channel, true);
  return STATUS_CODE_OK;
}

StatusCode bts_7040_get_measurement(Bts7040Storage *storage, uint16_t *measured) {
  AdcChannel sense_channel = NUM_ADC_CHANNELS;
  adc_get_channel(*storage->sense_pin, &sense_channel);
  adc_read_raw(sense_channel, measured);
  return STATUS_CODE_OK;
}
