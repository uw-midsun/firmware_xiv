#include "sense_temperature.h"

#include <stdbool.h>
#include <stddef.h>

#include "adc.h"
#include "data_store.h"
#include "gpio.h"
#include "log.h"
#include "sense.h"

typedef struct ThermistorData {
  DataPoint data_point;
  AdcChannel adc_channel;
} ThermistorData;

static ThermistorData s_thermistor_data[MAX_THERMISTORS];

static void prv_sense_callback(void *context) {
  // Just dump the raw ADC readings into the data store
  ThermistorData *data = context;
  uint16_t raw_reading;
  StatusCode status = adc_read_raw(data->adc_channel, &raw_reading);
  if (status_ok(status)) {
    data_store_set(data->data_point, raw_reading);
  } else {
    LOG_WARN("sense_temperature failed to read from ADC for data point %d: code %d\n",
             data->data_point, status);
  }
}

StatusCode sense_temperature_init(SenseTemperatureSettings *settings) {
  if (settings == NULL || settings->num_thermistors > MAX_THERMISTORS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  GpioSettings thermistor_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  for (uint8_t i = 0; i < settings->num_thermistors; i++) {
    ThermistorData *data = &s_thermistor_data[i];
    data->data_point = DATA_POINT_TEMPERATURE(i);
    status_ok_or_return(gpio_init_pin(&settings->thermistor_pins[i], &thermistor_pin_settings));
    status_ok_or_return(adc_get_channel(settings->thermistor_pins[i], &data->adc_channel));
    status_ok_or_return(adc_set_channel(data->adc_channel, true));
    status_ok_or_return(sense_register(prv_sense_callback, data));
  }

  return STATUS_CODE_OK;
}
