#include "solar_config.h"
#include "data_store.h"
#include "mcp3427_adc.h"
#include "solar_events.h"

#define SOLAR_MCP3427_CONVERSION_MODE MCP3427_CONVERSION_MODE_CONTINUOUS
#define SOLAR_MCP3427_SAMPLE_RATE MCP3427_SAMPLE_RATE_12_BIT

#define SOLAR_MCP3427_CURRENT_SENSE_AMP_GAIN MCP3427_AMP_GAIN_1
#define SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN MCP3427_AMP_GAIN_1

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

// |num_mcp3427s| is set dynamically by |config_get_sense_mcp3427_settings|
static const SenseMcp3427Settings s_base_sense_mcp3427_settings = {
  .mcp3427s =
      {
          // current sense
          {
              .mcp3427_settings =
                  {
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
                      .addr_pin_1 = MCP3427_PIN_STATE_LOW,
                      .amplifier_gain = SOLAR_MCP3427_CURRENT_SENSE_AMP_GAIN,
                      .conversion_mode = SOLAR_MCP3427_CONVERSION_MODE,
                      .port = I2C_PORT_1,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
              .data_point = DATA_POINT_CURRENT,
          },
          // voltage sense
          {
              .mcp3427_settings =
                  {
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
                      .addr_pin_1 = MCP3427_PIN_STATE_LOW,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .port = I2C_PORT_2,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
              .data_point = DATA_POINT_VOLTAGE_1,
          },
          {
              .mcp3427_settings =
                  {
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
                      .addr_pin_1 = MCP3427_PIN_STATE_FLOAT,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .port = I2C_PORT_2,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
              .data_point = DATA_POINT_VOLTAGE_2,
          },
          {
              .mcp3427_settings =
                  {
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
                      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .port = I2C_PORT_2,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
              .data_point = DATA_POINT_VOLTAGE_3,
          },
          {
              .mcp3427_settings =
                  {
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .addr_pin_0 = MCP3427_PIN_STATE_FLOAT,
                      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .port = I2C_PORT_2,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
              .data_point = DATA_POINT_VOLTAGE_4,
          },
          {
              .mcp3427_settings =
                  {
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .addr_pin_0 = MCP3427_PIN_STATE_HIGH,
                      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .port = I2C_PORT_2,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
              .data_point = DATA_POINT_VOLTAGE_5,
          },
          {
              // this one is not used on the 5 MPPT board, so it is last
              .mcp3427_settings =
                  {
                      .sample_rate = SOLAR_MCP3427_SAMPLE_RATE,
                      .addr_pin_0 = MCP3427_PIN_STATE_FLOAT,
                      .addr_pin_1 = MCP3427_PIN_STATE_LOW,
                      .amplifier_gain = SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN,
                      .port = I2C_PORT_2,
                      .adc_data_trigger_event = SOLAR_MCP3427_EVENT_DATA_TRIGGER,
                      .adc_data_ready_event = SOLAR_MCP3427_EVENT_DATA_READY,
                  },
              .data_point = DATA_POINT_VOLTAGE_6,
          },
      },
};

static const uint8_t s_board_type_to_num_mcp3427[] = {
  [SOLAR_BOARD_TYPE_5_MPPT] = 6,
  [SOLAR_BOARD_TYPE_6_MPPT] = 7,
};

StatusCode config_get_sense_mcp3427_settings(SolarBoardType board_type,
                                             SenseMcp3427Settings *settings) {
  if (board_type >= NUM_SOLAR_BOARD_TYPES || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *settings = s_base_sense_mcp3427_settings;
  settings->num_mcp3427s = s_board_type_to_num_mcp3427[board_type];
  return STATUS_CODE_OK;
}
