#include "output.h"

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "bts7040_load_switch.h"
#include "bts7200_load_switch.h"
#include "log.h"
#include "status.h"

// Experimentally determined, more accurate resistor value to convert BTS7200 sensed current
// (note that there are 1.6k resistors on the current rev as of 2020-10-31, but the scaling factor
// was experimentally determined to imply this resistor value)
#define POWER_DISTRIBUTION_BTS7200_SENSE_RESISTOR 1160

// Experimentally determined bias in the BTS7200 sensed output
#define POWER_DISTRIBUTION_BTS7200_BIAS (-8)

// All BTS7040s use a 1.21k resistor to convert sense current
#define POWER_DISTRIBUTION_BTS7040_SENSE_RESISTOR 1210

// To be calibrated - bias in the BTS7040 sensed output
#define POWER_DISTRIBUTION_BTS7040_BIAS 0

// Voltage at the SENSE pin is limited to a max of 3.3V by a diode.
// Due to to this function, since any fault current will be at least 4.4 mA (see p.g. 49)
// the resulting voltage will be 4.4 mA * 1.6 kOhm = ~7 V. Due to this,
// voltages approaching 3.3V represent a fault, and should be treated as such.
// Max doesn't matter much, so it's left as a high value to account for any errors.
#define POWER_DISTRIBUTION_BTS7200_MIN_FAULT_VOLTAGE_MV 3200
#define POWER_DISTRIBUTION_BTS7200_MAX_FAULT_VOLTAGE_MV 10000

// Apply similar logic for the BTS7040s (see p.g. 49 of BTS7040 datasheet):
#define POWER_DISTRIBUTION_BTS7040_MIN_FAULT_VOLTAGE_MV 3200
#define POWER_DISTRIBUTION_BTS7040_MAX_FAULT_VOLTAGE_MV 10000

// These are the number of BTS7200 and BTS7040 slots physically present on the board.
// Not all are used, some are only for spares.
// TODO(SOFT-396): should these be here?
#define NUM_BTS7200 9
#define NUM_BTS7040 7 // including 2 on UV cutoff

static Bts7200Storage s_bts7200_storage[NUM_BTS7200];
static uint8_t s_num_bts7200_storages;
static Bts7040Storage s_bts7040_storage[NUM_BTS7040];
static uint8_t s_num_bts7040_storages;

static union {
  Bts7200Storage *bts7200;
  Bts7040Storage *bts7040;
} s_output_to_storage[NUM_OUTPUTS] = { 0 };

static OutputConfig *s_config;

static StatusCode prv_init_gpio(OutputGpioSpec *spec) {
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  return gpio_init_pin(&spec->address, &settings);
}

static StatusCode prv_init_bts7200(Output output, OutputBts7200Spec *spec, bool is_front_pd) {
  // linear search to see if we've got the same bts7200 earlier
  // this makes initialization quadratic on the number of outputs but that's okay
  Bts7200Storage *storage = NULL;
  for (Output prev_output = 0; prev_output < output; prev_output++) {
    OutputSpec *prev_spec = &s_config->specs[prev_output];
    if (prev_spec->on_front != is_front_pd || prev_spec->type != OUTPUT_TYPE_BTS7200) continue;
    if (prev_spec->bts7200_spec.bts7200_info == spec->bts7200_info) {
      storage = s_output_to_storage[prev_output].bts7200;
      break;
    }
  }
  if (storage == NULL) {
    // we didn't find a previous output on the same bts7200 - use a new storage
    if (s_num_bts7200_storages >= NUM_BTS7200) {
      return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
    }
    storage = &s_bts7200_storage[s_num_bts7200_storages++];
  }
  s_output_to_storage[output] = storage;

  Bts7200Pca9539rSettings settings = {
    .enable_0_pin = &spec->bts7200_info->enable_0_pin,
    .enable_1_pin = &spec->bts7200_info->enable_1_pin,
    .select_pin = &spec->bts7200_info->dsel_pin,
    .sense_pin = &s_config->mux_output_pin,
    .resistor = POWER_DISTRIBUTION_BTS7200_SENSE_RESISTOR,
    .bias = POWER_DISTRIBUTION_BTS7200_BIAS,
    .min_fault_voltage_mv = POWER_DISTRIBUTION_BTS7200_MIN_FAULT_VOLTAGE_MV,
    .max_fault_voltage_mv = POWER_DISTRIBUTION_BTS7200_MAX_FAULT_VOLTAGE_MV,
  };
  return bts7200_init_pca9539r(storage, &settings);
}

static StatusCode prv_init_bts7040(Output output, OutputBts7040Spec *spec) {
  if (s_num_bts7040_storages >= NUM_BTS7040) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }
  Bts7040Storage *storage = &s_bts7040_storage[s_num_bts7040_storages++];
  s_output_to_storage[output] = storage;

  Bts7040Pca9539rSettings settings = {
    .enable_pin = &spec->enable_pin,
    .sense_pin = &s_config->mux_output_pin,
    .resistor = POWER_DISTRIBUTION_BTS7040_SENSE_RESISTOR,
    .bias = POWER_DISTRIBUTION_BTS7040_BIAS,
    .min_fault_voltage_mv = POWER_DISTRIBUTION_BTS7040_MIN_FAULT_VOLTAGE_MV,
    .max_fault_voltage_mv = POWER_DISTRIBUTION_BTS7040_MAX_FAULT_VOLTAGE_MV,
  };
  return bts7040_init_pca9539r(storage, &settings);
}

StatusCode output_init(OutputConfig *config, bool is_front_power_distro) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_config = config;
  s_num_bts7200_storages = 0;
  s_num_bts7040_storages = 0;

  // initialize PCA9539Rs, mux
  for (uint8_t i = 0; i < s_config->num_i2c_addresses; i++) {
    status_ok_or_return(pca9539r_gpio_init(config->i2c_port, config->i2c_addresses[i]));
  }
  mux_init(&s_config->mux_address);

  // initialize the mux enable pin to low - CD74HC4067M96's enable pin is active-low
  GpioSettings mux_enable_pin_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  status_ok_or_return(gpio_init_pin(&s_config->mux_enable_pin, &mux_enable_pin_settings));

  // initialize all the outputs on this board
  for (Output output = 0; output < NUM_OUTPUTS; output++) {
    OutputSpec *spec = &s_config->specs[output];
    if (spec->on_front != is_front_power_distro) {
      // not for this board 
      continue;
    }
    switch (spec->type) {
      case OUTPUT_TYPE_GPIO:
        status_ok_or_return(prv_init_gpio(&spec->gpio_spec));
        break;
      case OUTPUT_TYPE_BTS7200:
        status_ok_or_return(prv_init_bts7200(output, &spec->bts7200_spec, is_front_power_distro));
        break;
      case OUTPUT_TYPE_BTS7040:
        status_ok_or_return(prv_init_bts7040(output, &spec->bts7040_spec));
        break;
      default:
        // probably because it wasn't specified in the config: ignore with a warning
        LOG_WARN("Warning: output %d is unspecified\n", output);
        break;
    }
  }

  return STATUS_CODE_OK;
}

StatusCode output_set_state(Output output, OutputState state) {
  return STATUS_CODE_OK;
}

StatusCode output_read_current(Output output, uint16_t *current) {
  return STATUS_CODE_OK;
}
