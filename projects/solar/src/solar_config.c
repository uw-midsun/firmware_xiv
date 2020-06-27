#include "solar_config.h"
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
