#include "solar_config.h"

#include "data_store.h"
#include "gpio.h"
#include "mcp3427_adc.h"
#include "solar_events.h"

#define SOLAR_MCP3427_CONVERSION_MODE MCP3427_CONVERSION_MODE_CONTINUOUS
#define SOLAR_MCP3427_SAMPLE_RATE MCP3427_SAMPLE_RATE_12_BIT

static const Mcp3427Settings s_current_mcp3427_settings = {
  .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
  .addr_pin_0 = MCP3427_PIN_STATE_LOW,
  .addr_pin_1 = MCP3427_PIN_STATE_LOW,
  .amplifier_gain = MCP3427_AMP_GAIN_1,
  .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
  .port = I2C_PORT_1,
  .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
  .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
};

const SenseCurrentSettings sense_current_settings = {
  .current_mcp3427_settings = &s_current_mcp3427_settings,
};

// |num_thermistors| is set dynamically by |config_get_sense_temperature_settings|
static const SenseTemperatureSettings s_base_sense_temperature_settings =
    { .thermistor_pins = {
          { GPIO_PORT_A, 0 },
          { GPIO_PORT_A, 1 },
          { GPIO_PORT_A, 2 },
          { GPIO_PORT_A, 3 },
          { GPIO_PORT_A, 4 },
          { GPIO_PORT_A, 5 },  // not used on 5 MPPT board
      } };

StatusCode config_get_sense_temperature_settings(SolarMpptCount mppt_count,
                                                 SenseTemperatureSettings *settings) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *settings = s_base_sense_temperature_settings;
  settings->num_thermistors = mppt_count;  // we have one thermistor per MPPT
  return STATUS_CODE_OK;
}
