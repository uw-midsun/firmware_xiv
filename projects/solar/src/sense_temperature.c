#include "sense_temperature.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "adc.h"
#include "data_store.h"
#include "gpio.h"
#include "log.h"
#include "sense.h"

// log(x) in c is the mathematical ln(x).
#define ln(x) log(x)

typedef enum ThermistorType {
  NTC_THERMISTOR,
  RTD_THERMISTOR,
  FAN_CONTROL_THERMISTOR,
  NUM_THERMISTOR_TYPES,
} ThermistorType;

typedef struct ThermistorSettings {
  ThermistorType thermistor_type;
  GpioAddress thermistor_address;
} ThermistorSettings;

typedef struct ThermistorData {
  DataPoint data_point;
  AdcChannel adc_channel;
  ThermistorSettings thermistor_settings;
} ThermistorData;

static ThermistorData s_thermistor_data[MAX_THERMISTORS];

static void prv_sense_callback(void *context) {
  ThermistorData *data = context;
  uint16_t converted_reading;
  StatusCode status = adc_read_converted(data->adc_channel, &converted_reading);

  // Convert converted_reading into Volts
  converted_reading *= 1000;

  // Based on the type of thermistor, convert the converted reading (mV) to degrees Celcius
  switch (data->thermistor_settings.thermistor_type) {
    case NTC_THERMISTOR:
      // NTC: T = 1 / ( ( ln(0.56 * V / (3.3 - V)) / 3428 ) + 1/298.15 )
      converted_reading =
          1 / ((ln(0.56 * converted_reading / (3.3 - converted_reading)) / 3428.0) + 1.0 / 298.15);
      break;
    case RTD_THERMISTOR:
      // RTD: ΔT = (33 * V / (87.45 - V) - 1) / 0.00385
      converted_reading = (33.0 * converted_reading / (87.45 - converted_reading) - 1.0) / 0.00385;
      break;
    case FAN_CONTROL_THERMISTOR:
      // TODO(SOFT-280): Continuation, T = ΔV * q / (n * k * ln(10)), get two readings from fan and
      // convert readings.
      break;
    case NUM_THERMISTOR_TYPES:
      break;
  }

  if (status_ok(status)) {
    data_store_set(data->data_point, converted_reading);
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
