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

static StatusCode prv_init_bts7200(OutputBts7200Spec *spec) {
  // TODO(SOFT-396): look and see if this bts7200 has already been done
  // and maintain an output => bts7200 storage mapping
  return STATUS_CODE_OK;
}

static StatusCode prv_init_bts7040(OutputBts7040Spec *spec) {
  if (s_num_bts7040_storages >= NUM_BTS7040) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }
  Bts7040Storage *storage = &s_bts7040_storage[s_num_bts7040_storages++];
  Bts7040Pca9539rSettings settings = {
    .enable_pin = &spec->enable_pin,
    // .sense_pin = &s_hw_config.mux_output_pin, // TODO(SOFT-396): mux
    .resistor = POWER_DISTRIBUTION_BTS7040_SENSE_RESISTOR,
    .bias = POWER_DISTRIBUTION_BTS7040_BIAS,
    .min_fault_voltage_mv = POWER_DISTRIBUTION_BTS7040_MIN_FAULT_VOLTAGE_MV,
    .max_fault_voltage_mv = POWER_DISTRIBUTION_BTS7040_MAX_FAULT_VOLTAGE_MV,
  };
  return bts7040_init_pca9539r(storage, settings);
}

StatusCode output_init(OutputConfig *config, bool is_front_power_distro) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_config = config;
  s_num_bts7200_storages = 0;
  s_num_bts7040_storages = 0;

  // TODO(SOFT-396): initialize PCA9539R, mux

  for (Output output = 0; output < NUM_OUTPUTS; output++) {
    OutputSpec *spec = &s_config->specs[output];
    if (is_front_power_distro != spec->on_front) {
      // not for this board 
      continue;
    }
    switch (spec->type) {
      case OUTPUT_TYPE_GPIO:
        status_ok_or_return(prv_init_gpio(&spec->gpio_spec));
        break;
      case OUTPUT_TYPE_BTS7200:
        status_ok_or_return(prv_init_bts7200(&spec->bts7200_spec));
        break;
      case OUTPUT_TYPE_BTS7040:
        status_ok_or_return(prv_init_bts7040(&spec->bts7040_spec));
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
