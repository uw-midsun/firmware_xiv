#include "solar_config.h"

#include "can_msg_defs.h"
#include "data_store.h"
#include "data_tx.h"
#include "exported_enums.h"
#include "gpio.h"
#include "mcp3427_adc.h"
#include "pin_defs.h"
#include "solar_events.h"

// TODO(SOFT-282): Calibrate scaling factors and thresholds.

// Time between sense cycles (i.e. frequency of data update). 1 second.
#define SENSE_CYCLE_PERIOD_US 1000000

// Parameters for data_tx: we TX 8 messages at a time 100ms apart to avoid exhausting the CAN queue.
#define DATA_TX_WAIT_TIME_MS 100
#define DATA_TX_MSGS_PER_TX_ITERATION 8

// Scaling factor to convert MCP3427 ADC values (LSB = 62.5uV) for voltage sense to millivolts.
// Experimentally determined to be ~1.184mV/LSB.
#define SOLAR_MCP3427_VOLTAGE_SENSE_SCALING_FACTOR 1.184f

// Scaling factor from MCP3427 ADC values for current sense to microamps.
// MCP3427 uses 62.5uV/LSB, ACS722LLCTR-10AU has sensitivity of 264mV/A = 0.264uV/uA.
// (62.5uV/LSB)/(0.264uV/uA) = 236.74uA/LSB. Experimental value is 228.25uA/LSB, which is close.
#define SOLAR_MCP3427_CURRENT_SENSE_SCALING_FACTOR 228.25f

// Bias of MCP3427 ADC for current sense, in microamps.
// ACS722LLCTR-10AU gives a bias of 0.33V, corresponding to 1.25A.
// The experimental value of 1.2527A is close to this.
#define SOLAR_MCP3427_CURRENT_SENSE_BIAS 1.2527e6f

// Scaling factor to convert SPV1020 current values from SPI to microamps.
// This is based on ONE data point: reading 46 for a current of 600-800mA, so call it 700mA.
// 700mA/46LSB ~ 15220uA/LSB.
// TODO(SOFT-282): get more data points to calibrate this.
#define SOLAR_MPPT_CURRENT_SCALING_FACTOR 15220.0f

// Scaling factor to convert SPV1020 input voltage values from SPI to millivolts.
// Experimentally determined to be ~26.1mV/LSB.
#define SOLAR_MPPT_VIN_SCALING_FACTOR 26.1f

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

// Registers gpio interrupt with STATUS pin on drv120, enables fault handling
#define ENABLE_DRV120_FAULT_HANDLING true

// the number of MCP3427s above those associated 1:1 with MPPTs - currently, only current sense
#define NUM_EXTRA_NON_MPPT_MCP3427S 1

#define SOLAR_I2C_SPEED I2C_SPEED_FAST

static const I2CSettings s_i2c1_settings = {
  .speed = SOLAR_I2C_SPEED,
  .sda = SOLAR_I2C1_SDA,
  .scl = SOLAR_I2C1_SCL,
};

static const I2CSettings s_i2c2_settings = {
  .speed = SOLAR_I2C_SPEED,
  .sda = SOLAR_I2C2_SDA,
  .scl = SOLAR_I2C2_SCL,
};

static const SpiSettings s_spi_settings = {
  .baudrate = 60000,
  .mode = SPI_MODE_3,
  .mosi = SOLAR_SPI2_MOSI,
  .miso = SOLAR_SPI2_MISO,
  .sclk = SOLAR_SPI2_SCLK,
  .cs = SOLAR_UNUSED_PIN,
};

static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_SOLAR,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = SOLAR_CAN_EVENT_RX,
  .tx_event = SOLAR_CAN_EVENT_TX,
  .fault_event = SOLAR_CAN_EVENT_FAULT,
  .rx = SOLAR_CAN_RX_PIN,
  .tx = SOLAR_CAN_TX_PIN,
  .loopback = false,
};

static const GpioAddress s_drv120_relay_pin = { GPIO_PORT_A, 8 };
static const GpioAddress s_drv120_status_pin = { GPIO_PORT_A, 6 };

static const SenseSettings s_sense_settings = {
  .sense_period_us = SENSE_CYCLE_PERIOD_US,
};

static const FaultHandlerSettings s_fault_handler_settings = {
  .relay_open_faults =
      {
          EE_SOLAR_FAULT_OVERCURRENT,
          EE_SOLAR_FAULT_NEGATIVE_CURRENT,
          EE_SOLAR_FAULT_OVERVOLTAGE,
      },
  .num_relay_open_faults = 3,
};

static const DataTxSettings s_data_tx_settings = {
  .wait_between_tx_in_millis = DATA_TX_WAIT_TIME_MS,
  .msgs_per_tx_iteration = DATA_TX_MSGS_PER_TX_ITERATION,
};

const I2CSettings *config_get_i2c1_settings(void) {
  return &s_i2c1_settings;
}

const I2CSettings *config_get_i2c2_settings(void) {
  return &s_i2c2_settings;
}

const SpiSettings *config_get_spi_settings(void) {
  return &s_spi_settings;
}

const CanSettings *config_get_can_settings(void) {
  return &s_can_settings;
}

const GpioAddress *config_get_drv120_enable_pin(void) {
  return &s_drv120_relay_pin;
}

const GpioAddress *config_get_drv120_status_pin(void) {
  return ENABLE_DRV120_FAULT_HANDLING ? &s_drv120_status_pin : NULL;
}

const SenseSettings *config_get_sense_settings(void) {
  return &s_sense_settings;
}

const FaultHandlerSettings *config_get_fault_handler_settings(void) {
  return &s_fault_handler_settings;
}

const DataTxSettings *config_get_data_tx_settings(void) {
  return &s_data_tx_settings;
}

// |num_mcp3427s| is set dynamically by |config_get_sense_mcp3427_settings|
static SenseMcp3427Settings s_sense_mcp3427_settings = {
  .mcp3427s =
      {
          // current sense
          {
              .data_point = DATA_POINT_CURRENT,
              .scaling_factor = SOLAR_MCP3427_CURRENT_SENSE_SCALING_FACTOR,
              .bias = SOLAR_MCP3427_CURRENT_SENSE_BIAS,
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

const SenseMcp3427Settings *config_get_sense_mcp3427_settings(SolarMpptCount mppt_count) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return NULL;
  }
  s_sense_mcp3427_settings.num_mcp3427s = mppt_count + NUM_EXTRA_NON_MPPT_MCP3427S;
  return &s_sense_mcp3427_settings;
}

// |num_thermistors| is set dynamically by |config_get_sense_temperature_settings|
static SenseTemperatureSettings s_sense_temperature_settings =
    { .thermistor_pins = {
          { GPIO_PORT_A, 0 },
          { GPIO_PORT_A, 1 },
          { GPIO_PORT_A, 2 },
          { GPIO_PORT_A, 3 },
          { GPIO_PORT_A, 4 },
          { GPIO_PORT_A, 5 },  // not used on 5 MPPT board
      } };

const SenseTemperatureSettings *config_get_sense_temperature_settings(SolarMpptCount mppt_count) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return NULL;
  }
  s_sense_temperature_settings.num_thermistors = mppt_count;  // we have one thermistor per MPPT
  return &s_sense_temperature_settings;
}

// |mppt_count| is set dynamically by |config_get_sense_mppt_settings|
static SenseMpptSettings s_sense_mppt_settings = {
  .spi_port = SPI_PORT_2,
  .mppt_current_scaling_factor = SOLAR_MPPT_CURRENT_SCALING_FACTOR,
  .mppt_vin_scaling_factor = SOLAR_MPPT_VIN_SCALING_FACTOR,
};

const SenseMpptSettings *config_get_sense_mppt_settings(SolarMpptCount mppt_count) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return NULL;
  }
  s_sense_mppt_settings.mppt_count = mppt_count;
  return &s_sense_mppt_settings;
}

// |mppt_count| is set dynamically by |config_get_fault_monitor_settings|
static FaultMonitorSettings s_fault_monitor_settings = {
  .output_overcurrent_threshold_uA = SOLAR_OUTPUT_OVERCURRENT_THRESHOLD_uA,
  .output_overvoltage_threshold_mV = SOLAR_OUTPUT_OVERVOLTAGE_THRESHOLD_mV,
  .overtemperature_threshold_dC = SOLAR_OVERTEMPERATURE_THRESHOLD_dC,
};

const FaultMonitorSettings *config_get_fault_monitor_settings(SolarMpptCount mppt_count) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return NULL;
  }
  s_fault_monitor_settings.mppt_count = mppt_count;
  return &s_fault_monitor_settings;
}

static FanControlSettingsSolar s_fan_control_settings = {
  .overtemp_addr = { .port = GPIO_PORT_B, .pin = 5 },
  .fan_fail_addr = { .port = GPIO_PORT_B, .pin = 7 },
  .full_speed_addr = { .port = GPIO_PORT_B, .pin = 6 },
  .full_speed_temp_threshold = SOLAR_OVERTEMPERATURE_THRESHOLD_dC,
};

const FanControlSettingsSolar *config_get_fan_control_settings(SolarMpptCount mppt_count) {
  if (mppt_count > MAX_SOLAR_BOARD_MPPTS) {
    return NULL;
  }
  s_fan_control_settings.mppt_count = mppt_count;

  return &s_fan_control_settings;
}
