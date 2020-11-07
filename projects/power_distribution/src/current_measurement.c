#include "current_measurement.h"

#include <stddef.h>
#include "bts7040_load_switch.h"
#include "bts7200_load_switch.h"
#include "log.h"

static PowerDistributionCurrentHardwareConfig s_hw_config;
static PowerDistributionCurrentStorage s_storage = { 0 };
static Bts7200Storage s_bts7200_storages[MAX_POWER_DISTRIBUTION_BTS7200_CHANNELS];
static Bts7040Storage s_bts7040_storages[MAX_POWER_DISTRIBUTION_BTS7040_CHANNELS];
static SoftTimerId s_timer_id;

static uint32_t s_interval_us;
static PowerDistributionCurrentMeasurementCallback s_callback;
static void *s_callback_context;

static void prv_measure_currents(SoftTimerId timer_id, void *context) {
  // read from all the BTS7200s
  for (uint8_t i = 0; i < s_hw_config.num_bts7200_channels; i++) {
    mux_set(&s_hw_config.mux_address, s_hw_config.bts7200s[i].mux_selection);
    bts7200_get_measurement(&s_bts7200_storages[i],
                            &s_storage.measurements[s_hw_config.bts7200s[i].current_0],
                            &s_storage.measurements[s_hw_config.bts7200s[i].current_1]);
  }

  // read from all the BTS7040s
  for (uint8_t i = 0; i < s_hw_config.num_bts7040_channels; i++) {
    mux_set(&s_hw_config.mux_address, s_hw_config.bts7040s[i].mux_selection);
    // bts7040_get_measurement(&s_bts7040_storages[i],
    //                         &s_storage.measurements[s_hw_config.bts7040s[i].current]);
  }

  if (s_callback) {
    s_callback(s_callback_context);
  }

  soft_timer_start(s_interval_us, &prv_measure_currents, NULL, &s_timer_id);
}

StatusCode power_distribution_current_measurement_init(PowerDistributionCurrentSettings *settings) {
  if (settings->hw_config.num_bts7200_channels > MAX_POWER_DISTRIBUTION_BTS7200_CHANNELS ||
      settings->hw_config.num_bts7040_channels > MAX_POWER_DISTRIBUTION_BTS7040_CHANNELS) {
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

  // initialize the mux enable pin to low - CD74HC4067M96's enable pin is active-low
  GpioSettings mux_enable_pin_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  status_ok_or_return(gpio_init_pin(&s_hw_config.mux_enable_pin, &mux_enable_pin_settings));

  // note: we don't have to initialize the mux_output_pin as ADC because
  // bts7200_init_pca9539r and bts7040_init_pca9539r do it for us

  // initialize and start the BTS7200s
  Bts7200Pca9539rSettings bts7200_settings = {
    .sense_pin = &s_hw_config.mux_output_pin,
    .resistor = POWER_DISTRIBUTION_BTS7200_SENSE_RESISTOR,
    .bias = POWER_DISTRIBUTION_BTS7200_BIAS,
    .min_fault_voltage_mv = POWER_DISTRIBUTION_BTS7200_MIN_FAULT_VOLTAGE_MV,
    .max_fault_voltage_mv = POWER_DISTRIBUTION_BTS7200_MAX_FAULT_VOLTAGE_MV,
  };

  for (uint8_t i = 0; i < s_hw_config.num_bts7200_channels; i++) {
    // check that the currents are valid
    if (s_hw_config.bts7200s[i].current_0 >= NUM_POWER_DISTRIBUTION_CURRENTS ||
        s_hw_config.bts7200s[i].current_1 >= NUM_POWER_DISTRIBUTION_CURRENTS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }

    // add DSEL, EN0, EN1 pins
    bts7200_settings.select_pin = &s_hw_config.bts7200s[i].dsel_pin;
    bts7200_settings.enable_0_pin = &s_hw_config.bts7200s[i].en0_pin;
    bts7200_settings.enable_1_pin = &s_hw_config.bts7200s[i].en1_pin;

    status_ok_or_return(bts7200_init_pca9539r(&s_bts7200_storages[i], &bts7200_settings));
  }

  // initialize all BTS7040/7008s
  Bts7040Pca9539rSettings bts7040_settings = {
    .sense_pin = &s_hw_config.mux_output_pin,
    .resistor = POWER_DISTRIBUTION_BTS7040_SENSE_RESISTOR,
    .bias = POWER_DISTRIBUTION_BTS7040_BIAS,
    .min_fault_voltage_mv = POWER_DISTRIBUTION_BTS7040_MIN_FAULT_VOLTAGE_MV,
    .max_fault_voltage_mv = POWER_DISTRIBUTION_BTS7040_MAX_FAULT_VOLTAGE_MV,
  };
  for (uint8_t i = 0; i < s_hw_config.num_bts7040_channels; i++) {
    // check that the current is valid
    if (s_hw_config.bts7040s[i].current >= NUM_POWER_DISTRIBUTION_CURRENTS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }

    // add EN pin
    bts7040_settings.enable_pin = &s_hw_config.bts7040s[i].en_pin;

    status_ok_or_return(bts7040_init_pca9539r(&s_bts7040_storages[i], &bts7040_settings));
  }

  // measure the currents immediately; the callback doesn't use the timer id it's passed
  prv_measure_currents(SOFT_TIMER_INVALID_TIMER, NULL);

  return STATUS_CODE_OK;
}

PowerDistributionCurrentStorage *power_distribution_current_measurement_get_storage(void) {
  return &s_storage;
}

StatusCode power_distribution_current_measurement_stop(void) {
  soft_timer_cancel(s_timer_id);
  s_timer_id = SOFT_TIMER_INVALID_TIMER;

  return STATUS_CODE_OK;
}
