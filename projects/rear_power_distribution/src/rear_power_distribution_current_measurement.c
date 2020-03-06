#include "rear_power_distribution_current_measurement.h"

#include <stddef.h>
#include "bts_7200_current_sense.h"

static RearPowerDistributionCurrentHardwareConfig s_hw_config;
static RearPowerDistributionCurrentStorage s_storage = { 0 };
static Bts7200Storage s_bts7200_storages[MAX_REAR_POWER_DISTRIBUTION_CURRENTS];
static SoftTimerId s_timer_id;

static uint32_t s_interval_us;
static RearPowerDistributionCurrentMeasurementCallback s_callback;
static void *s_callback_context;

static void prv_measure_currents(SoftTimerId timer_id, void *context) {
  // read from all the BTS7200s in order
  for (uint8_t i = 0; i < s_hw_config.num_bts7200_channels; i++) {
    mux_set(&s_hw_config.mux_address, s_hw_config.bts7200_to_mux_select[i]);
    bts_7200_get_measurement(&s_bts7200_storages[i], &s_storage.measurements[2 * i],
                             &s_storage.measurements[2 * i + 1]);
  }

  if (s_callback) {
    s_callback(s_callback_context);
  }

  soft_timer_start(s_interval_us, &prv_measure_currents, NULL, &s_timer_id);
}

StatusCode rear_power_distribution_current_measurement_init(
    RearPowerDistributionCurrentSettings *settings) {
  if (settings->hw_config.num_dsel_i2c_addresses > MAX_I2C_DSEL_ADDRESSES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_hw_config = settings->hw_config;
  s_interval_us = settings->interval_us;
  s_callback = settings->callback;
  s_callback_context = settings->callback_context;

  // initialize the PCA9539R on every I2C address specified
  for (uint8_t i = 0; i < s_hw_config.num_dsel_i2c_addresses; i++) {
    status_ok_or_return(
        pca9539r_gpio_init(s_hw_config.i2c_port, s_hw_config.dsel_i2c_addresses[i]));
  }
  status_ok_or_return(mux_init(&s_hw_config.mux_address));

  // note: we don't have to initialize the mux_output_pin as ADC because
  // bts_7200_init_pca9539r does it for us

  // initialize and start the BTS7200s
  Bts7200Pca9539rSettings bts_7200_settings = {
    .sense_pin = &s_hw_config.mux_address.mux_output_pin,
    .interval_us = s_interval_us,
  };
  for (uint8_t i = 0; i < s_hw_config.num_bts7200_channels; i++) {
    bts_7200_settings.select_pin = &s_hw_config.bts7200_to_dsel_address[i];
    status_ok_or_return(bts_7200_init_pca9539r(&s_bts7200_storages[i], &bts_7200_settings));
  }

  // measure the currents immediately; the callback doesn't use the timer it's passed
  prv_measure_currents(SOFT_TIMER_INVALID_TIMER, NULL);

  return STATUS_CODE_OK;
}

RearPowerDistributionCurrentStorage *rear_power_distribution_current_measurement_get_storage(void) {
  return &s_storage;
}

StatusCode rear_power_distribution_stop(void) {
  soft_timer_cancel(s_timer_id);
  s_timer_id = SOFT_TIMER_INVALID_TIMER;
  return STATUS_CODE_OK;
}
