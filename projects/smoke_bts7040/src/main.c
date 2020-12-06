// A simple smoke test for BTS7040

// Periodically take reading from user selected channels and log the result
// Configurable items: wait time, FRONT or REAR power distro selection, channels to be tested

#include "bts7040_load_switch.h"
#include "controller_board_pins.h"
#include "current_measurement_config.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"
#include "soft_timer.h"
#include "wait.h"

// experimentally determined value: scaling factor of 670/1160 ~ 0.578 ohms, bias of -8 mA
#define SMOKE_BTS7040_SENSE_RESISTOR 1160
#define SMOKE_BTS7040_BIAS (-8)

#define SMOKE_BTS7040_MIN_FAULT_VOLTAGE_MV 3200
#define SMOKE_BTS7040_MAX_FAULT_VOLTAGE_MV 10000

// Smoke test settings. Can be modified to fit testing purpose.
#define CURRENT_MEASURE_INTERVAL_MS 500  // Set wait time between each set of readings
#define IS_FRONT_POWER_DISTRO true       // Set whether to test FRONT or REAR power distro

// Maximum number of channels that can be tested,
// which is the same number of bts7200 used in front/rear power distribution
#define MAX_TEST_CHANNELS 8

#define I2C_PORT I2C_PORT_2
#define PIN_I2C_SCL CONTROLLER_BOARD_ADDR_I2C2_SCL
#define PIN_I2C_SDA CONTROLLER_BOARD_ADDR_I2C2_SDA

// Set of channels to be tested, the array contains the indices of the BTS7200 in the front/rear HW
// config Details at
// https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/1419740028/BTS7200+smoke+test+user+guide
static uint8_t s_test_channels[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

static Bts7040Storage s_bts7040_storages[MAX_TEST_CHANNELS];

static void prv_read_and_log(SoftTimerId timer_id, void *context) {
  PowerDistributionCurrentHardwareConfig *s_hw_config = context;
  uint16_t current;

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_test_channels); i++) {
    mux_set(&s_hw_config->mux_address, s_hw_config->bts7040s[s_test_channels[i]].mux_selection);
    bts7040_get_measurement(&s_bts7040_storages[i], &current);

    LOG_DEBUG("Channel: %d; current: %d\n", s_test_channels[i], current);
  }
  soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_read_and_log, s_hw_config, NULL);
}

static void prv_fault_callback(void *context) {
  LOG_DEBUG("Fault detected!\n");
}

int main() {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = PIN_I2C_SDA,
    .scl = PIN_I2C_SCL,
  };
  i2c_init(I2C_PORT, &i2c_settings);

  PowerDistributionCurrentHardwareConfig s_hw_config =
      IS_FRONT_POWER_DISTRO ? FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG
                            : REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG;

  status_ok_or_return(mux_init(&s_hw_config.mux_address));

  // Initialize the mux enable pin to low - CD74HC4067M96's enable pin is active low on power distro
  GpioSettings mux_enable_pin_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  status_ok_or_return(gpio_init_pin(&s_hw_config.mux_enable_pin, &mux_enable_pin_settings));

  // Initialize and start the BTS7040s
  Bts7040Pca9539rSettings bts7040_settings = {
    .sense_pin = &s_hw_config.mux_output_pin,
    .i2c_port = s_hw_config.i2c_port,
    .fault_callback = prv_fault_callback,
    .resistor = SMOKE_BTS7040_SENSE_RESISTOR,
    .bias = SMOKE_BTS7040_BIAS,
    .min_fault_voltage_mv = SMOKE_BTS7040_MIN_FAULT_VOLTAGE_MV,
    .max_fault_voltage_mv = SMOKE_BTS7040_MAX_FAULT_VOLTAGE_MV,
  };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_test_channels); i++) {
    bts7040_settings.enable_pin = &s_hw_config.bts7040s[s_test_channels[i]].en_pin;
    status_ok_or_return(bts7040_init_pca9539r(&s_bts7040_storages[i], &bts7040_settings));
    bts7040_enable_output(&s_bts7040_storages[i]);
  }

  soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_read_and_log, &s_hw_config, NULL);
  while (true) {
    wait();
  }

  return 0;
}
