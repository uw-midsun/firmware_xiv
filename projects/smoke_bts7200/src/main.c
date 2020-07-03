// A simple smoke test for BTS7200

#include "bts_7200_current_sense.h"
#include "current_measurement.h"
#include "current_measurement_config.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "mux.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

#define CURRENT_MEASURE_INTERVAL_MS 500
#define IS_FRONT_POWER_DISTRO true

#define NUM_TEST_CHANNELS 3
static uint8_t selected_channels[NUM_TEST_CHANNELS] = { 1, 2, 3 };

static Bts7200Storage s_bts7200_storages[NUM_TEST_CHANNELS];

static void prv_read_and_log(SoftTimerId timer_id, void *context) {
  PowerDistributionCurrentHardwareConfig *s_hw_config = context;
  uint16_t current_0, current_1;
  StatusCode status = STATUS_CODE_OK;

  for (uint8_t i = 0; i < SIZEOF_ARRAY(selected_channels); i++) {
    mux_set(&s_hw_config->mux_address, s_hw_config->bts7200s[selected_channels[i]].mux_selection);
    bts_7200_get_measurement(&s_bts7200_storages[i], &current_0, &current_1);

    if (status == STATUS_CODE_OK)
      LOG_DEBUG("Channel: %d; current_0: %d, current_1: %d\n", selected_channels[i], current_0,
                current_1);
    else
      LOG_DEBUG("Fail to take current reading\n");
  }
  soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_read_and_log, NULL, NULL);
}

int main() {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  PowerDistributionCurrentHardwareConfig s_hw_config =
      IS_FRONT_POWER_DISTRO ? FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG
                            : REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG;

  Bts7200Pca9539rSettings bts_7200_settings = {
    .sense_pin = &s_hw_config.mux_address.mux_output_pin,
  };
  for (uint8_t i = 0; i < NUM_TEST_CHANNELS; i++) {
    // check that the currents are valid
    if (s_hw_config.bts7200s[selected_channels[i]].current_0 >= NUM_POWER_DISTRIBUTION_CURRENTS ||
        s_hw_config.bts7200s[selected_channels[i]].current_1 >= NUM_POWER_DISTRIBUTION_CURRENTS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }

    bts_7200_settings.select_pin = &s_hw_config.bts7200s[selected_channels[i]].dsel_pin;
    status_ok_or_return(bts_7200_init_pca9539r(&s_bts7200_storages[i], &bts_7200_settings));
  }

  soft_timer_start_millis(CURRENT_MEASURE_INTERVAL_MS, prv_read_and_log, NULL, NULL);
  while (true) {
    wait();
  }

  return 0;
}
