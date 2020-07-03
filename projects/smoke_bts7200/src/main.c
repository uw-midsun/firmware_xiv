// A simple smoke test for BTS7200

// Periodically take reading from user selected channels and log the result
// Configurable items: wait time, FRONT or REAR power distro selection, channels to be tested
#include "bts_7200_current_sense.h"
#include "current_measurement.h"
#include "current_measurement_config.h"

#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "mux.h"
#include "soft_timer.h"
#include "wait.h"

// Smoke test settings. Can be modified to fit testing purpose.
#define CURRENT_MEASURE_INTERVAL_MS 500  // Set wait time between each set of readings
#define IS_FRONT_POWER_DISTRO true       // Set whether to test FRONT or REAR power distro

// Number of channels to be tested, MUST match the number of elements in test_channels
#define NUM_TEST_CHANNELS 8
// Set of channels to be tested, the number of elements MUST match NUM_TEST_CHANNELS.
// Range of channels: 0-7
static uint8_t test_channels[NUM_TEST_CHANNELS] = { 0, 1, 2, 3, 4, 5, 6, 7 };

static Bts7200Storage s_bts7200_storages[NUM_TEST_CHANNELS];

static void prv_read_and_log(SoftTimerId timer_id, void *context) {
  PowerDistributionCurrentHardwareConfig *s_hw_config = context;
  uint16_t current_0, current_1;

  for (uint8_t i = 0; i < SIZEOF_ARRAY(test_channels); i++) {
    mux_set(&s_hw_config->mux_address, s_hw_config->bts7200s[test_channels[i]].mux_selection);
    bts_7200_get_measurement(&s_bts7200_storages[i], &current_0, &current_1);

    LOG_DEBUG("Channel: %d; current_0: %d, current_1: %d\n", test_channels[i], current_0,
              current_1);
  }
  soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_read_and_log, s_hw_config, NULL);
}

int main() {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  PowerDistributionCurrentHardwareConfig s_hw_config =
      IS_FRONT_POWER_DISTRO ? FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG
                            : REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG;

  // Initialize the PCA9539R on every I2C address specified
  for (uint8_t i = 0; i < s_hw_config.num_dsel_i2c_addresses; i++) {
    status_ok_or_return(
        pca9539r_gpio_init(s_hw_config.i2c_port, s_hw_config.dsel_i2c_addresses[i]));
  }
  status_ok_or_return(mux_init(&s_hw_config.mux_address));

  // Initialize and start the BTS7200s
  Bts7200Pca9539rSettings bts_7200_settings = {
    .sense_pin = &s_hw_config.mux_address.mux_output_pin,
  };

  for (uint8_t i = 0; i < NUM_TEST_CHANNELS; i++) {
    // check that the currents are valid
    if (s_hw_config.bts7200s[test_channels[i]].current_0 >= NUM_POWER_DISTRIBUTION_CURRENTS ||
        s_hw_config.bts7200s[test_channels[i]].current_1 >= NUM_POWER_DISTRIBUTION_CURRENTS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }

    bts_7200_settings.select_pin = &s_hw_config.bts7200s[test_channels[i]].dsel_pin;
    status_ok_or_return(bts_7200_init_pca9539r(&s_bts7200_storages[i], &bts_7200_settings));
  }

  soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_read_and_log, &s_hw_config, NULL);
  while (true) {
    wait();
  }

  return 0;
}
