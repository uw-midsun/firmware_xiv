// A simple smoke test for BTS7040

// Periodically take reading from user selected channels and log the result
// Configurable items: wait time, FRONT or REAR power distro selection, channels to be tested
#include "bts_7040_7008_current_sense.h"
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

// Maximum number of channels that can be tested,
// which is the same number of bts7200 used in front/rear power distribution
#define MAX_TEST_CHANNELS 8
// Set of channels to be tested, the array contains the indices of the BTS7200 in the front/rear HW
// config Details at
static uint8_t s_test_channels[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

static Bts7040Storage s_bts7040_storages[MAX_TEST_CHANNELS];

static void prv_read_and_log(SoftTimerId timer_id, void *context) {
  PowerDistributionCurrentHardwareConfig *s_hw_config = context;
  uint16_t current;

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_test_channels); i++) {
    mux_set(&s_hw_config->mux_address, s_hw_config->bts7040s[s_test_channels[i]].mux_selection);
    bts_7040_get_measurement(&s_bts7040_storages[i], &current);

    LOG_DEBUG("Channel: %d; current: %d\n", s_test_channels[i], current);
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

  status_ok_or_return(mux_init(&s_hw_config.mux_address));

  // Initialize and start the BTS7040s
  Bts7040Settings bts_7040_settings = {
    .sense_pin = &s_hw_config.mux_address.mux_output_pin,
  };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_test_channels); i++) {
    status_ok_or_return(bts_7040_init(&s_bts7040_storages[i], &bts_7040_settings));
  }

  soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_read_and_log, &s_hw_config, NULL);
  while (true) {
    wait();
  }

  return 0;
}
