#include "output.h"

#include <stdbool.h>
#include <stdint.h>

#include "bts7040_load_switch.h"
#include "bts7200_load_switch.h"
#include "can_transmit.h"
#include "gpio.h"
#include "log.h"
#include "pca9539r_gpio_expander.h"
#include "pd_error_defs.h"
#include "status.h"

// Experimentally determined, more accurate resistor value to convert BTS7200 sensed current
// (note that there are 1.6k resistors on the current rev as of 2020-10-31, but the scaling factor
// was experimentally determined to imply this resistor value)
#define BTS7200_SENSE_RESISTOR 1160

// Experimentally determined bias in the BTS7200 sensed output
#define BTS7200_BIAS (-8)

// All BTS7040s use a 1.21k resistor to convert sense current
#define BTS7040_SENSE_RESISTOR 1210

// To be calibrated - bias in the BTS7040 sensed output
#define BTS7040_BIAS 0

// Voltage at the SENSE pin is limited to a max of 3.3V by a diode.
// Due to to this function, since any fault current will be at least 4.4 mA (see Table 21, p.g. 49)
// the resulting voltage will be 4.4 mA * 1.6 kOhm = ~7 V. Due to this,
// voltages approaching 3.3V represent a fault, and should be treated as such.
// Max doesn't matter much, so it's left as a high value to account for any errors.
#define BTS7200_MIN_FAULT_VOLTAGE_MV 3200

// Apply similar logic for the BTS7040s, see Table 21 (p.g. 49) in BTS7040 datasheet
#define BTS7040_MIN_FAULT_VOLTAGE_MV 3200

// These are the number of BTS7200 and BTS7040 slots physically present on the board.
// Not all are used, some are only for spares.
#define NUM_BTS7200 9
#define NUM_BTS7040 7  // including 2 on UV cutoff

static Bts7200Storage s_bts7200_storage[NUM_BTS7200];
static uint8_t s_num_bts7200_storages;
static Bts7040Storage s_bts7040_storage[NUM_BTS7040];
static uint8_t s_num_bts7040_storages;

static union {
  Bts7200Storage *bts7200;
  Bts7040Storage *bts7040;
} s_output_to_storage[NUM_OUTPUTS] = { 0 };

static OutputConfig *s_config = NULL;
static bool s_is_front_power_distro;

static void prv_tx_fault(uint16_t fault_flag, Output output) {
  if (s_is_front_power_distro) {
    CAN_TRANSMIT_FRONT_PD_FAULT(fault_flag, output);
  } else {
    CAN_TRANSMIT_REAR_PD_FAULT(fault_flag, 0, 0, output);
  }
}

static void prv_bts7200_fault_callback(Bts7200Channel channel, void *context) {
  Output output = (Output)(uintptr_t)context;  // retrieve the output stuffed into the context
  LOG_WARN("BTS7200 fault on output %d! Attempting recovery...\n", output);
  prv_tx_fault(BTS7200_FAULT, output);
}

static void prv_bts7040_fault_callback(void *context) {
  Output output = (Output)(uintptr_t)context;
  LOG_WARN("BTS7040 fault on output %d! Attempting recovery...\n", output);
  prv_tx_fault(BTS7040_FAULT, output);
}

static StatusCode prv_init_gpio(OutputGpioSpec *spec) {
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  return gpio_init_pin(&spec->address, &settings);
}

static StatusCode prv_init_bts7200(Output output, OutputBts7200Spec *spec) {
  if (spec->bts7200_info == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // linear search to see if we've got the same bts7200 earlier
  // this makes initialization quadratic on the number of outputs but that's okay
  Bts7200Storage *storage = NULL;
  for (Output prev_output = 0; prev_output < output; prev_output++) {
    OutputSpec *prev_spec = &s_config->specs[prev_output];
    if (prev_spec->on_front != s_is_front_power_distro || prev_spec->type != OUTPUT_TYPE_BTS7200) {
      continue;
    }
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
  s_output_to_storage[output].bts7200 = storage;

  Bts7200Pca9539rSettings settings = {
    .enable_0_pin = &spec->bts7200_info->enable_0_pin,
    .enable_1_pin = &spec->bts7200_info->enable_1_pin,
    .select_pin = &spec->bts7200_info->dsel_pin,
    .sense_pin = &s_config->mux_output_pin,
    .resistor = BTS7200_SENSE_RESISTOR,
    .bias = BTS7200_BIAS,
    .min_fault_voltage_mv = BTS7200_MIN_FAULT_VOLTAGE_MV,
    .fault_callback = prv_bts7200_fault_callback,
    .fault_callback_context = (void *)(uintptr_t)output,  // stuff the output into a void pointer
  };
  return bts7200_init_pca9539r(storage, &settings);
}

static StatusCode prv_init_bts7040(Output output, OutputBts7040Spec *spec) {
  if (s_num_bts7040_storages >= NUM_BTS7040) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }
  Bts7040Storage *storage = &s_bts7040_storage[s_num_bts7040_storages++];
  s_output_to_storage[output].bts7040 = storage;

  Bts7040Pca9539rSettings settings = {
    .enable_pin = &spec->enable_pin,
    .sense_pin = &s_config->mux_output_pin,
    .resistor = BTS7040_SENSE_RESISTOR,
    .bias = BTS7040_BIAS,
    .min_fault_voltage_mv = BTS7040_MIN_FAULT_VOLTAGE_MV,
    .fault_callback = prv_bts7040_fault_callback,
    .fault_callback_context = (void *)(uintptr_t)output,
  };
  return bts7040_init_pca9539r(storage, &settings);
}

StatusCode output_init(OutputConfig *config, bool is_front_power_distro) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_config = config;
  s_is_front_power_distro = is_front_power_distro;
  s_num_bts7200_storages = 0;
  s_num_bts7040_storages = 0;

  // initialize PCA9539Rs, mux
  for (uint8_t i = 0; i < s_config->num_i2c_addresses; i++) {
    status_ok_or_return(pca9539r_gpio_init(config->i2c_port, config->i2c_addresses[i]));
  }
  status_ok_or_return(mux_init(&s_config->mux_address));

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
        status_ok_or_return(prv_init_bts7200(output, &spec->bts7200_spec));
        break;
      case OUTPUT_TYPE_BTS7040:
        status_ok_or_return(prv_init_bts7040(output, &spec->bts7040_spec));
        break;
      default:
        // probably because it wasn't specified in the config: just ignore it
        break;
    }
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_set_state_gpio(GpioAddress *address, OutputState state) {
  // assume active-high always
  GpioState gpio_state = (state == OUTPUT_STATE_ON) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  return gpio_set_state(address, gpio_state);
}

static StatusCode prv_set_state_bts7xxx(Bts7xxxEnablePin *en_pin, OutputState state) {
  // again assume active-high
  if (state == OUTPUT_STATE_ON) {
    return bts7xxx_enable_pin(en_pin);
  } else {
    return bts7xxx_disable_pin(en_pin);
  }
}

static StatusCode prv_set_state_bts7200(Output output, OutputState state) {
  OutputSpec *spec = &s_config->specs[output];
  Bts7200Storage *storage = s_output_to_storage[output].bts7200;
  Bts7xxxEnablePin *en_pin =
      (spec->bts7200_spec.channel == 0) ? &storage->enable_pin_0 : &storage->enable_pin_1;
  return prv_set_state_bts7xxx(en_pin, state);
}

StatusCode output_set_state(Output output, OutputState state) {
  if (s_config == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }
  if (output >= NUM_OUTPUTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  OutputSpec *spec = &s_config->specs[output];
  switch (spec->type) {
    case OUTPUT_TYPE_GPIO:
      return prv_set_state_gpio(&spec->gpio_spec.address, state);
    case OUTPUT_TYPE_BTS7040:
      return prv_set_state_bts7xxx(&s_output_to_storage[output].bts7040->enable_pin, state);
    case OUTPUT_TYPE_BTS7200:
      return prv_set_state_bts7200(output, state);
    default:
      LOG_WARN("Warning: output %d is unspecified, not setting to on=%d\n", output, state);
      return STATUS_CODE_INVALID_ARGS;
  }
}

static StatusCode prv_read_current_bts7040(Output output, uint16_t *current) {
  uint8_t mux_selection = s_config->specs[output].bts7040_spec.mux_selection;
  status_ok_or_return(mux_set(&s_config->mux_address, mux_selection));
  return bts7040_get_measurement(s_output_to_storage[output].bts7040, current);
}

static StatusCode prv_read_current_bts7200(Output output, uint16_t *current) {
  uint8_t mux_selection = s_config->specs[output].bts7200_spec.bts7200_info->mux_selection;
  status_ok_or_return(mux_set(&s_config->mux_address, mux_selection));
  return bts7200_get_measurement(s_output_to_storage[output].bts7200, current,
                                 s_config->specs[output].bts7200_spec.channel);
}

StatusCode output_read_current(Output output, uint16_t *current) {
  if (s_config == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }
  if (output >= NUM_OUTPUTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  switch (s_config->specs[output].type) {
    case OUTPUT_TYPE_GPIO:
      // we can't current sense from a gpio output - UV_VBAT_IS uses BTS7040 (really a BTS7004)
      return STATUS_CODE_UNIMPLEMENTED;
    case OUTPUT_TYPE_BTS7040:
      return prv_read_current_bts7040(output, current);
    case OUTPUT_TYPE_BTS7200:
      return prv_read_current_bts7200(output, current);
    default:
      LOG_WARN("Warning: output %d is unspecified, not reading current\n", output);
      return STATUS_CODE_INVALID_ARGS;
  }
}

Bts7200Storage *test_output_get_bts7200_storage(Output output) {
  if (s_config == NULL || s_config->specs[output].type != OUTPUT_TYPE_BTS7200) {
    return NULL;
  }
  return s_output_to_storage[output].bts7200;
}
