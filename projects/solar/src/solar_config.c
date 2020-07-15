#include "solar_config.h"

#include "data_store.h"
#include "gpio.h"
#include "mcp3427_adc.h"
#include "solar_events.h"

#define SOLAR_MCP3427_CONVERSION_MODE MCP3427_CONVERSION_MODE_CONTINUOUS
#define SOLAR_MCP3427_SAMPLE_RATE MCP3427_SAMPLE_RATE_12_BIT

#define SOLAR_MCP3427_CURRENT_SENSE_AMP_GAIN MCP3427_AMP_GAIN_1
#define SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN MCP3427_AMP_GAIN_1

// the number of MCP3427s above those associated 1:1 with MPPTs - currently, only current sense
#define NUM_EXTRA_NON_MPPT_MCP3427S 1

// |num_mcp3427s| is set dynamically by |config_get_sense_mcp3427_settings|
static const SenseMcp3427Settings s_base_sense_mcp3427_settings = {
  .mcp3427s =
      {
          // current sense
          {
              .data_point = DATA_POINT_CURRENT,
              .mcp3427_settings =
                  {
                      .port = I2C_PORT_1,
                      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
                      .addr_pin_1 = MCP3427_PIN_STATE_LOW,
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .amplifier_gain = SOLAR_MCP3427_CURRENT_SENSE_AMP_GAIN,
                      .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
          },
          // voltage sense
          {
              .data_point = DATA_POINT_VOLTAGE_1,
              .mcp3427_settings =
                  {
                      .port = I2C_PORT_2,
                      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
                      .addr_pin_1 = MCP3427_PIN_STATE_LOW,
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
          },
          {
              .data_point = DATA_POINT_VOLTAGE_2,
              .mcp3427_settings =
                  {
                      .port = I2C_PORT_2,
                      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
                      .addr_pin_1 = MCP3427_PIN_STATE_FLOAT,
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
          },
          {
              .data_point = DATA_POINT_VOLTAGE_3,
              .mcp3427_settings =
                  {
                      .port = I2C_PORT_2,
                      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
                      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
          },
          {
              .data_point = DATA_POINT_VOLTAGE_4,
              .mcp3427_settings =
                  {
                      .port = I2C_PORT_2,
                      .addr_pin_0 = MCP3427_PIN_STATE_FLOAT,
                      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
          },
          {
              .data_point = DATA_POINT_VOLTAGE_5,
              .mcp3427_settings =
                  {
                      .port = I2C_PORT_2,
                      .addr_pin_0 = MCP3427_PIN_STATE_HIGH,
                      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
          },
          {
              // this one is not used on the 5 MPPT board, so it is last
              .data_point = DATA_POINT_VOLTAGE_6,
              .mcp3427_settings =
                  {
                      .port = I2C_PORT_2,
                      .addr_pin_0 = MCP3427_PIN_STATE_FLOAT,
                      .addr_pin_1 = MCP3427_PIN_STATE_LOW,
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
          },
      },
};

StatusCode config_get_sense_mcp3427_settings(SolarMpptCount mppt_count,
                                             SenseMcp3427Settings *settings) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *settings = s_base_sense_mcp3427_settings;
  settings->num_mcp3427s = mppt_count + NUM_EXTRA_NON_MPPT_MCP3427S;
  return STATUS_CODE_OK;
}

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
