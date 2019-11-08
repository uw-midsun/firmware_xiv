#include "thermistor.h"
#include <limits.h>
#include <stdint.h>
#include "log.h"

// src:
// https://www.murata.com/en-global/products/productdata/8796836626462/NTHCG83.txt
// Expected resistance in milliohms for a given temperature in celsius
//
// This table covers the range [0 C, 100 C] in 1 degree steps
static const uint32_t s_resistance_lookup[] = {
  27218600, 26076000, 24987700, 23950900, 22962900, 22021100, 21123000, 20266600, 19449500,
  18669800, 17925500, 17213900, 16534400, 15885600, 15265800, 14673500, 14107500, 13566400,
  13048900, 12554000, 12080500, 11628100, 11194700, 10779500, 10381500, 10000000, 9634200,
  9283500,  8947000,  8624200,  8314500,  8018100,  7733700,  7460900,  7199100,  6947900,
  6706700,  6475100,  6252600,  6039000,  5833600,  5635700,  5445400,  5262300,  5086300,
  4916900,  4753900,  4597100,  4446100,  4300800,  4160900,  4026200,  3896400,  3771400,
  3651000,  3535000,  3423100,  3315200,  3211300,  3111000,  3014300,  2922400,  2833700,
  2748200,  2665700,  2586100,  2509300,  2435100,  2363500,  2294300,  2227500,  2162700,
  2100100,  2039600,  1981100,  1924500,  1869800,  1817000,  1765800,  1716400,  1668500,
  1622400,  1577700,  1534500,  1492700,  1452100,  1412900,  1374900,  1338100,  1302500,
  1268000,  1234300,  1201600,  1170000,  1139300,  1109600,  1080700,  1052800,  1025600,
  999300,   973800,
};

// Used for converting the lookup table index with corresponding temperatures
#define THERMISTOR_LOOKUP_RANGE (SIZEOF_ARRAY(s_resistance_lookup) - 1)

#define THERMISTOR_MILLIOHMS_TO_OHMS(x) ((x) / 1000)
#define THERMISTOR_DECICELSIUS_TO_CELSIUS(x) ((x) / 10)
#define THERMISTOR_CELSIUS_TO_DECICELSIUS(x) ((x)*10)

StatusCode thermistor_init(ThermistorStorage *storage, GpioAddress thermistor_gpio,
                           ThermistorPosition position) {
  storage->position = position;
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&thermistor_gpio, &gpio_settings);

  adc_get_channel(thermistor_gpio, &(storage->adc_channel));
  adc_set_channel(storage->adc_channel, true);
  return STATUS_CODE_OK;
}

StatusCode thermistor_get_temp(ThermistorStorage *storage, uint16_t *temperature_dc) {
  // Fetch the voltage readings
  uint16_t reading = 0;                     // the divided voltage in millivolts
  uint32_t thermistor_resistance_ohms = 0;  // resistance in milliohms
  uint16_t vdda = 0;                        // vdda voltage in millivolts

  // Get source voltage and node voltage of the voltage divider
  status_ok_or_return(adc_read_converted(ADC_CHANNEL_REF, &vdda));
  status_ok_or_return(adc_read_converted(storage->adc_channel, &reading));

  if (vdda == 0) {
    *temperature_dc = UINT16_MAX;
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "No source voltage detected.");
  } else if (reading == 0) {
    *temperature_dc = UINT16_MAX;
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "No node voltage detected.");
  }

  // Calculates thermistor resistance in milliOhms based on the position of
  // thermistor and the fixed resistor
  if (storage->position == THERMISTOR_POSITION_R1) {
    thermistor_resistance_ohms =
        ((uint32_t)(vdda - reading)) * (uint32_t)THERMISTOR_FIXED_RESISTANCE_OHMS / reading;
  } else {
    thermistor_resistance_ohms =
        ((uint32_t)THERMISTOR_FIXED_RESISTANCE_OHMS * reading) / (uint32_t)(vdda - reading);
  }
  return thermistor_calculate_temp(thermistor_resistance_ohms, (uint16_t *)temperature_dc);
}

StatusCode thermistor_calculate_temp(uint32_t thermistor_resistance_ohms,
                                     uint16_t *temperature_dc) {
  // Find the approximate target temperature from the arguments passed
  for (uint16_t i = 0; i < SIZEOF_ARRAY(s_resistance_lookup) - 1; i++) {
    if (thermistor_resistance_ohms * 1000 <= s_resistance_lookup[i] &&
        thermistor_resistance_ohms * 1000 >= s_resistance_lookup[i + 1]) {
      // Return the temperature with the linear approximation in deciCelsius
      *temperature_dc = (uint16_t)(
          ((uint32_t)i * 1000 + ((s_resistance_lookup[i] - thermistor_resistance_ohms * 1000) *
                                 1000 / (s_resistance_lookup[i] - s_resistance_lookup[i + 1]))) /
          100);
      return STATUS_CODE_OK;
    }
  }
  // Sets the returned temperature to be absurdly large
  *temperature_dc = UINT16_MAX;
  return status_msg(STATUS_CODE_OUT_OF_RANGE, "Temperature out of lookup table range.");
}

StatusCode thermistor_calculate_resistance(uint16_t temperature_dc,
                                           uint16_t *thermistor_resistor_ohms) {
  if (temperature_dc > THERMISTOR_CELSIUS_TO_DECICELSIUS(THERMISTOR_LOOKUP_RANGE)) {
    return status_msg(STATUS_CODE_OUT_OF_RANGE,
                      "Input temperature, exceeds lookup table ranges (0-100 deg).");
  } else if (temperature_dc == THERMISTOR_CELSIUS_TO_DECICELSIUS(THERMISTOR_LOOKUP_RANGE)) {
    // For the higher lookup edge case
    *thermistor_resistor_ohms =
        THERMISTOR_MILLIOHMS_TO_OHMS(s_resistance_lookup[THERMISTOR_LOOKUP_RANGE]);
  } else {
    // Linearly interpolate between the two points and then convert to Ohms
    uint16_t lower_temp = THERMISTOR_DECICELSIUS_TO_CELSIUS(temperature_dc);
    *thermistor_resistor_ohms = THERMISTOR_MILLIOHMS_TO_OHMS(
        (s_resistance_lookup[lower_temp] * 10 +
         (s_resistance_lookup[lower_temp + 1] - s_resistance_lookup[lower_temp]) *
             (temperature_dc % 10)) /
        10);
  }

  return STATUS_CODE_OK;
}
