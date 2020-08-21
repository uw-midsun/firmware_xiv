#include "solar_config.h"

#include "can_msg_defs.h"
#include "data_store.h"
#include "exported_enums.h"
#include "gpio.h"
#include "mcp3427_adc.h"
#include "solar_events.h"

// TODO(SOFT-282): Calibrate scaling factors and thresholds.

// Time between sense cycles (i.e. frequency of data update). 1 second.
#define SENSE_CYCLE_PERIOD_US 1000000

// Scaling factor to convert MCP3427 ADC values (LSB = 62.5uV) for voltage sense to millivolts.
// Must be calibrated.
#define SOLAR_MCP3427_VOLTAGE_SENSE_SCALING_FACTOR 1.0f

// Scaling factor from MCP3427 ADC values for current sense to microamps.
// MCP3427 uses 62.5uV/LSB, ACS722LLCTR-10AU has sensitivity of 264mV/A = 0.264uV/uA.
// (62.5uV/LSB)/(0.264uV/uA) = 236.74uA/LSB
#define SOLAR_MCP3427_CURRENT_SENSE_SCALING_FACTOR 236.74f

// Scaling factor to convert SPV1020 current values from SPI to microamps.
// Must be calibrated.
#define SOLAR_MPPT_CURRENT_SCALING_FACTOR 1.0f

// Scaling factor to convert SPV1020 VIN values from SPI to millivolts.
// Must be calibrated.
#define SOLAR_MPPT_VIN_SCALING_FACTOR 1.0f

// Overcurrent threshold for the output current of the array. 9A.
#define SOLAR_OUTPUT_OVERCURRENT_THRESHOLD_uA 9000000

// Overvoltage threshold for the sum of the output voltages of the MPPTs. 160V.
// Note that this sums 5 MPPTs' output voltages on the 5 MPPT board and 6 on the 6 MPPT board.
#define SOLAR_OUTPUT_OVERVOLTAGE_THRESHOLD_mV 160000

// Overtemperature threshold for any individual thermistor. To be calibrated, currently 100C.
#define SOLAR_OVERTEMPERATURE_THRESHOLD_dC 1000

#define SOLAR_MCP3427_CONVERSION_MODE MCP3427_CONVERSION_MODE_CONTINUOUS
#define SOLAR_MCP3427_SAMPLE_RATE MCP3427_SAMPLE_RATE_12_BIT

#define SOLAR_MCP3427_CURRENT_SENSE_AMP_GAIN MCP3427_AMP_GAIN_1
#define SOLAR_MCP3427_VOLTAGE_SENSE_AMP_GAIN MCP3427_AMP_GAIN_1

// the number of MCP3427s above those associated 1:1 with MPPTs - currently, only current sense
#define NUM_EXTRA_NON_MPPT_MCP3427S 1

#define SOLAR_I2C_SPEED I2C_SPEED_FAST

#define I2C1_SDA \
  { GPIO_PORT_B, 11 }
#define I2C1_SCL \
  { GPIO_PORT_B, 10 }
#define I2C2_SDA \
  { GPIO_PORT_B, 9 }
#define I2C2_SCL \
  { GPIO_PORT_B, 8 }

#define SPI2_MOSI \
  { GPIO_PORT_B, 15 }
#define SPI2_MISO \
  { GPIO_PORT_B, 14 }
#define SPI2_SCLK \
  { GPIO_PORT_B, 13 }
#define CS_UNUSED \
  { GPIO_PORT_B, 1 }

#define CAN_RX_PIN \
  { GPIO_PORT_A, 12 }
#define CAN_TX_PIN \
  { GPIO_PORT_A, 11 }

const I2CSettings i2c1_settings = {
  .speed = SOLAR_I2C_SPEED,
  .sda = I2C1_SDA,
  .scl = I2C1_SCL,
};

const I2CSettings i2c2_settings = {
  .speed = SOLAR_I2C_SPEED,
  .sda = I2C2_SDA,
  .scl = I2C2_SCL,
};

const SpiSettings spi_settings = {
  .baudrate = 60000,
  .mode = SPI_MODE_3,
  .mosi = SPI2_MOSI,
  .miso = SPI2_MISO,
  .sclk = SPI2_SCLK,
  .cs = CS_UNUSED,
};

const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_SOLAR,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = SOLAR_CAN_EVENT_RX,
  .tx_event = SOLAR_CAN_EVENT_TX,
  .fault_event = SOLAR_CAN_EVENT_FAULT,
  .rx = CAN_RX_PIN,
  .tx = CAN_TX_PIN,
  .loopback = false,
};

const GpioAddress drv120_relay_pin = { GPIO_PORT_A, 8 };

const SenseSettings sense_settings = {
  .sense_period_us = SENSE_CYCLE_PERIOD_US,
};

const FaultHandlerSettings fault_handler_settings = {
  .relay_open_faults =
      {
          EE_SOLAR_FAULT_OVERCURRENT,
          EE_SOLAR_FAULT_NEGATIVE_CURRENT,
          EE_SOLAR_FAULT_OVERVOLTAGE,
      },
  .num_relay_open_faults = 3,
};

// |num_mcp3427s| is set dynamically by |config_get_sense_mcp3427_settings|
static const SenseMcp3427Settings s_base_sense_mcp3427_settings = {
  .mcp3427s =
      {
          // current sense
          {
              .data_point = DATA_POINT_CURRENT,
              .scaling_factor = SOLAR_MCP3427_CURRENT_SENSE_SCALING_FACTOR,
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
              .data_point = DATA_POINT_VOLTAGE(0),
              .scaling_factor = SOLAR_MCP3427_VOLTAGE_SENSE_SCALING_FACTOR,
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
              .data_point = DATA_POINT_VOLTAGE(1),
              .scaling_factor = SOLAR_MCP3427_VOLTAGE_SENSE_SCALING_FACTOR,
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
              .data_point = DATA_POINT_VOLTAGE(2),
              .scaling_factor = SOLAR_MCP3427_VOLTAGE_SENSE_SCALING_FACTOR,
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
              .data_point = DATA_POINT_VOLTAGE(3),
              .scaling_factor = SOLAR_MCP3427_VOLTAGE_SENSE_SCALING_FACTOR,
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
              .data_point = DATA_POINT_VOLTAGE(4),
              .scaling_factor = SOLAR_MCP3427_VOLTAGE_SENSE_SCALING_FACTOR,
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
              .data_point = DATA_POINT_VOLTAGE(5),
              .scaling_factor = SOLAR_MCP3427_VOLTAGE_SENSE_SCALING_FACTOR,
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

// |mppt_count| is set dynamically by |config_get_sense_mppt_settings|
static const SenseMpptSettings s_base_sense_mppt_settings = {
  .spi_port = SPI_PORT_2,
  .mppt_current_scaling_factor = SOLAR_MPPT_CURRENT_SCALING_FACTOR,
  .mppt_vin_scaling_factor = SOLAR_MPPT_VIN_SCALING_FACTOR,
};

StatusCode config_get_sense_mppt_settings(SolarMpptCount mppt_count, SenseMpptSettings *settings) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *settings = s_base_sense_mppt_settings;
  settings->mppt_count = mppt_count;
  return STATUS_CODE_OK;
}

// |mppt_count| is set dynamically by |config_get_fault_monitor_settings|
static const FaultMonitorSettings s_base_fault_monitor_settings = {
  .output_overcurrent_threshold_uA = SOLAR_OUTPUT_OVERCURRENT_THRESHOLD_uA,
  .output_overvoltage_threshold_mV = SOLAR_OUTPUT_OVERVOLTAGE_THRESHOLD_mV,
  .overtemperature_threshold_dC = SOLAR_OVERTEMPERATURE_THRESHOLD_dC,
};

StatusCode config_get_fault_monitor_settings(SolarMpptCount mppt_count,
                                             FaultMonitorSettings *settings) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *settings = s_base_fault_monitor_settings;
  settings->mppt_count = mppt_count;
  return STATUS_CODE_OK;
}
