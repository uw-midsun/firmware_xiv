#include "rear_power_distribution_current_measurement.h"

#include <stddef.h>
#include "bts_7200_current_sense.h"

// These are declared in the header
const GpioAddress REAR_POWER_DISTRIBUTION_CURRENT_I2C_SDA_ADDRESS  //
    = { .port = GPIO_PORT_B, .pin = 11 };
const GpioAddress REAR_POWER_DISTRIBUTION_CURRENT_I2C_SCL_ADDRESS  //
    = { .port = GPIO_PORT_B, .pin = 10 };

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
    sn74_mux_set(&s_hw_config.mux_address, s_hw_config.bts7200_to_mux_select[i]);
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
  s_hw_config = settings->hw_config;
  s_interval_us = settings->interval_us;
  s_callback = settings->callback;
  s_callback_context = settings->callback_context;

  status_ok_or_return(mcp23008_gpio_init(s_hw_config.dsel_i2c_address));
  status_ok_or_return(sn74_mux_init_mux(&s_hw_config.mux_address));

  // note: we don't have to initialize the mux_output_pin as ADC because
  // bts_7200_init_mcp23008 does it for us

  // initialize and start the BTS7200s
  Bts7200Mcp23008Settings bts_7200_settings = {
    .sense_pin = &s_hw_config.mux_address.mux_output_pin,
    .interval_us = s_interval_us,
  };
  for (uint8_t i = 0; i < s_hw_config.num_bts7200_channels; i++) {
    bts_7200_settings.select_pin = &s_hw_config.bts7200_to_dsel_address[i];
    status_ok_or_return(bts_7200_init_mcp23008(&s_bts7200_storages[i], &bts_7200_settings));
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
