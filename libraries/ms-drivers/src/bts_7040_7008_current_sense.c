#include "bts_7040_7008_current_sense.h"
#include "adc.h"

StatusCode bts_7040_init(Bts7040Settings *settings, Bts7040Storage *storage) {
  storage->sense_pin = settings->sense_pin;

  // enable the enable pin
  Pca9539rGpioSettings enable_settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,
    .state = PCA9539R_GPIO_STATE_HIGH,
  };
  status_ok_or_return(pca9539r_gpio_init_pin(settings->enable_pin, &enable_settings));

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
