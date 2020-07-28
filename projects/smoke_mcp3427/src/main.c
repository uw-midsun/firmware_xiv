// A simple smoke test for MCP3427_AMP_GAIN_1.

// Log readings whenever readings are available from the voltage/current sense MCP3427s.
// Configurable items: I2C settings, MCP3427 device settings, which MCP3427s to be tested.
// More detailed guide can be found here:
// https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/1475543041/MCP3427+smoke+test+user+guide
#include "mcp3427_adc.h"

#include "event_queue.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "status.h"
#include "wait.h"

// Which MCP3427 to read from. By default channel 1 according to the schematics.
#define SENSE_MCP3427_CHANNEL MCP3427_CHANNEL_1

// I2C settings
#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 11 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 10 }
#define I2C_PORT I2C_PORT_1  // I2C_PORT_2

// MCP3427 device settings
#define SMOKE_MAX_CONSECUTIVE_FAULTS 3
#define SMOKE_SAMPLE_RATE MCP3427_SAMPLE_RATE_12_BIT
#define SMOKE_AMP_GAIN MCP3427_AMP_GAIN_1
#define SMOKE_CONVERSION_MODE MCP3427_CONVERSION_MODE_ONE_SHOT

// Unique ID of each MCP3427
typedef enum {
  // Voltages from the voltage sense MCP3427s
  MCP3427_VOLTAGE_1 = 0,
  MCP3427_VOLTAGE_2,
  MCP3427_VOLTAGE_3,
  MCP3427_VOLTAGE_4,
  MCP3427_VOLTAGE_5,
  MCP3427_VOLTAGE_6,

  // Current from the current sense MCP3427s
  MCP3427_CURRENT,
} SmokeDataPoint;

typedef enum {
  MCP3427_DATA_TRIGGER_EVENT = 0,
  MCP3427_DATA_READY_EVENT,
} SmokeMcp3427Event;

// Hold smoke test data of each MCP3427
typedef struct SmokeMcp3427Data {
  Mcp3427Storage mcp3427_storage;
  SmokeDataPoint mcp3427_data_point;

  uint8_t consecutive_faults;
} SmokeMcp3427Data;

#define MAX_NUM_MCP3427 7
// Store the settings of each MCP3427
static Mcp3427Settings s_mcp3427_configs[MAX_NUM_MCP3427] = {
  // Voltage sense mcp3427 settings
  {
      .sample_rate = SMOKE_SAMPLE_RATE,
      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
      .addr_pin_1 = MCP3427_PIN_STATE_LOW,
      .amplifier_gain = SMOKE_AMP_GAIN,
      .conversion_mode = SMOKE_CONVERSION_MODE,
      .adc_data_ready_event = MCP3427_DATA_READY_EVENT,
      .adc_data_trigger_event = MCP3427_DATA_TRIGGER_EVENT,
  },
  {
      .sample_rate = SMOKE_SAMPLE_RATE,
      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
      .addr_pin_1 = MCP3427_PIN_STATE_FLOAT,
      .amplifier_gain = SMOKE_AMP_GAIN,
      .conversion_mode = SMOKE_CONVERSION_MODE,
      .adc_data_ready_event = MCP3427_DATA_READY_EVENT,
      .adc_data_trigger_event = MCP3427_DATA_TRIGGER_EVENT,
  },
  {
      .sample_rate = SMOKE_SAMPLE_RATE,
      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
      .amplifier_gain = SMOKE_AMP_GAIN,
      .conversion_mode = SMOKE_CONVERSION_MODE,
      .adc_data_ready_event = MCP3427_DATA_READY_EVENT,
      .adc_data_trigger_event = MCP3427_DATA_TRIGGER_EVENT,
  },
  {
      .sample_rate = SMOKE_SAMPLE_RATE,
      .addr_pin_0 = MCP3427_PIN_STATE_HIGH,
      .addr_pin_1 = MCP3427_PIN_STATE_FLOAT,
      .amplifier_gain = SMOKE_AMP_GAIN,
      .conversion_mode = SMOKE_CONVERSION_MODE,
      .adc_data_ready_event = MCP3427_DATA_READY_EVENT,
      .adc_data_trigger_event = MCP3427_DATA_TRIGGER_EVENT,
  },
  {
      .sample_rate = SMOKE_SAMPLE_RATE,
      .addr_pin_0 = MCP3427_PIN_STATE_HIGH,
      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
      .amplifier_gain = SMOKE_AMP_GAIN,
      .conversion_mode = SMOKE_CONVERSION_MODE,
      .adc_data_ready_event = MCP3427_DATA_READY_EVENT,
      .adc_data_trigger_event = MCP3427_DATA_TRIGGER_EVENT,
  },
  {
      .sample_rate = SMOKE_SAMPLE_RATE,
      .addr_pin_0 = MCP3427_PIN_STATE_FLOAT,
      .addr_pin_1 = MCP3427_PIN_STATE_HIGH,
      .amplifier_gain = SMOKE_AMP_GAIN,
      .conversion_mode = SMOKE_CONVERSION_MODE,
      .adc_data_ready_event = MCP3427_DATA_READY_EVENT,
      .adc_data_trigger_event = MCP3427_DATA_TRIGGER_EVENT,
  },
  // Current sense mcp3427 settings
  {
      .sample_rate = SMOKE_SAMPLE_RATE,
      .addr_pin_0 = MCP3427_PIN_STATE_LOW,
      .addr_pin_1 = MCP3427_PIN_STATE_LOW,
      .amplifier_gain = SMOKE_AMP_GAIN,
      .conversion_mode = SMOKE_CONVERSION_MODE,
      .adc_data_ready_event = MCP3427_DATA_READY_EVENT,
      .adc_data_trigger_event = MCP3427_DATA_TRIGGER_EVENT,
  }
};

static SmokeMcp3427Data s_mcp3427_data[MAX_NUM_MCP3427];
// s_test_devices: hold the indices of specific mcp3427s being tested.
// For testing voltage sense mcp3427, MUST use indices 0 to 5.
// For testing current sense mcp3427, MUST use index 6
static uint8_t s_test_devices[] = { 0, 1, 2, 3, 4, 5 };  // { 6 }

static void prv_mcp3427_callback(int16_t value_ch1, int16_t value_ch2, void *context) {
  SmokeMcp3427Data *data = context;
  int16_t value = (SENSE_MCP3427_CHANNEL == MCP3427_CHANNEL_1) ? value_ch1 : value_ch2;
  if (data->mcp3427_data_point < MCP3427_CURRENT) {
    LOG_DEBUG("Voltage sense mcp3427 ID = %d; Value = %d\n", data->mcp3427_data_point, value);
  } else {
    LOG_DEBUG("Current sense mcp3427 ID = %d; Value = %d\n", data->mcp3427_data_point, value);
  }
  data->consecutive_faults = 0;
}

static void prv_mcp3527_fault_callback(void *context) {
  SmokeMcp3427Data *data = context;
  data->consecutive_faults++;
  if (data->consecutive_faults >= SMOKE_MAX_CONSECUTIVE_FAULTS) {
    if (data->mcp3427_data_point < MCP3427_CURRENT) {
      LOG_WARN("Voltage sense mcp3427 ID = %d encountered too many faults\n",
               data->mcp3427_data_point);
    } else {
      LOG_WARN("Current sense mcp3427 ID = %d encountered too many faults",
               data->mcp3427_data_point);
    }
    data->consecutive_faults = 0;
  }
}

int main() {
  interrupt_init();
  gpio_init();

  I2CSettings i2c1_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C1_SDA,
    .scl = I2C1_SCL,
  };
  i2c_init(I2C_PORT, &i2c1_settings);

  for (size_t i = 0; i < SIZEOF_ARRAY(s_test_devices); i++) {
    SmokeMcp3427Data *data = &s_mcp3427_data[s_test_devices[i]];
    data->mcp3427_data_point = s_test_devices[i];
    data->consecutive_faults = 0;
    status_ok_or_return(
        mcp3427_register_callback(&data->mcp3427_storage, prv_mcp3427_callback, data));
    status_ok_or_return(
        mcp3427_register_fault_callback(&data->mcp3427_storage, prv_mcp3527_fault_callback, data));
    status_ok_or_return(
        mcp3427_init(&data->mcp3427_storage, &s_mcp3427_configs[s_test_devices[i]]));
  }

  for (size_t i = 0; i < SIZEOF_ARRAY(s_test_devices); i++) {
    status_ok_or_return(mcp3427_start(&s_mcp3427_data[s_test_devices[i]].mcp3427_storage));
  }

  while (true) {
    Event e = { 0 };
    while (event_process(&e) == STATUS_CODE_OK) {
      mcp3427_process_event(&e);
    }
    wait();
  }
  return STATUS_CODE_OK;
}
