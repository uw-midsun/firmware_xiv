#include "pd_gpio.h"

#include "bts7xxx_common.h"
#include "current_measurement.h"
#include "log.h"
#include "pca9539r_gpio_expander.h"
#include "pd_events.h"
#include "status.h"

static PowerDistributionGpioConfig s_config;

// TODO(SOFT-336): make this not the worst thing ever.
// Set the address using the bts7xxx driver for fault handling logic.
static StatusCode prv_set_pin(Pca9539rGpioAddress *addr, Pca9539rGpioState state) {
  // extract the enable pins directly from current_measurement :(
  size_t num_bts7xxx = 0;
  Bts7xxxEnablePin **en_pins = power_distribution_current_measurement_get_pins(&num_bts7xxx);

  // now we linear search to find the enable pin struct :((
  Bts7xxxEnablePin *en_pin = NULL;
  for (size_t i = 0; i < num_bts7xxx; i++) {
    // straight up assuming that they're all on pca9539r
    Pca9539rGpioAddress *other_addr = en_pins[i]->enable_pin_pca9539r;
    if (other_addr == NULL) continue;

    if (addr->i2c_address == other_addr->i2c_address && addr->pin == other_addr->pin) {
      en_pin = en_pins[i];
      break;
    }
  }

  if (en_pin == NULL) {
    // the enable pin was not found on this earth
    LOG_WARN("Trying to change pin 0x%x / IO%d_%d, but its load switch enable pin wasn't found\n",
             addr->i2c_address, addr->pin / 8, addr->pin % 8);
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }
  // LOG_DEBUG("setting state %d en_pin addr 0x%x pin IO%d_%d\n", state, addr->i2c_address, addr->pin / 8, addr->pin % 8);
  if (state == PCA9539R_GPIO_STATE_LOW) {
    return bts7xxx_disable_pin(en_pin);
  } else {
    return bts7xxx_enable_pin(en_pin);
  }
}

StatusCode power_distribution_gpio_init(PowerDistributionGpioConfig config) {
  s_config = config;

  // initialize all the output pins
  Pca9539rGpioSettings settings = {
    .direction = PCA9539R_GPIO_DIR_OUT,
  };
  for (uint8_t i = 0; i < s_config.num_addresses; i++) {
    PowerDistributionGpioState state = s_config.all_addresses_and_default_states[i].state;

    if (state != POWER_DISTRIBUTION_GPIO_STATE_LOW && state != POWER_DISTRIBUTION_GPIO_STATE_HIGH) {
      // the only valid default states are low and high (no same/opposite/invalid)
      return status_code(STATUS_CODE_INVALID_ARGS);
    }

    settings.state = (state == POWER_DISTRIBUTION_GPIO_STATE_LOW) ? PCA9539R_GPIO_STATE_LOW
                                                                  : PCA9539R_GPIO_STATE_HIGH;

    status_ok_or_return(
        pca9539r_gpio_init_pin(&s_config.all_addresses_and_default_states[i].address, &settings));
  }

  // catch all invalid states here for fast failure
  for (uint8_t i = 0; i < s_config.num_events; i++) {
    for (uint8_t j = 0; j < s_config.events[i].num_outputs; j++) {
      if (s_config.events[i].outputs[j].state >= NUM_POWER_DISTRIBUTION_GPIO_STATES) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
    }
  }

  return STATUS_CODE_OK;
}

StatusCode power_distribution_gpio_process_event(Event *e) {
  PowerDistributionGpioEvent id = e->id;

  // find the event spec or ignore if it isn't there
  PowerDistributionGpioEventSpec *event_spec = NULL;
  for (uint8_t i = 0; i < s_config.num_events; i++) {
    if (s_config.events[i].event_id == id) {
      event_spec = &s_config.events[i];
      break;
    }
  }
  if (!event_spec) {
    // silently ignore unspecified events - it wasn't meant for us
    return STATUS_CODE_OK;
  }

  LOG_DEBUG("acting on spec id %d\n", id);
  // act on all the event spec's outputs
  for (uint8_t i = 0; i < event_spec->num_outputs; i++) {
    PowerDistributionGpioOutputSpec *output_spec = &event_spec->outputs[i];

    Pca9539rGpioState state;
    switch (output_spec->state) {
      case POWER_DISTRIBUTION_GPIO_STATE_LOW:
        state = PCA9539R_GPIO_STATE_LOW;
        break;
      case POWER_DISTRIBUTION_GPIO_STATE_HIGH:
        state = PCA9539R_GPIO_STATE_HIGH;
        break;
      case POWER_DISTRIBUTION_GPIO_STATE_SAME_AS_DATA:
        state = (e->data == 0) ? PCA9539R_GPIO_STATE_LOW : PCA9539R_GPIO_STATE_HIGH;
        break;
      case POWER_DISTRIBUTION_GPIO_STATE_OPPOSITE_TO_DATA:
        state = (e->data == 0) ? PCA9539R_GPIO_STATE_HIGH : PCA9539R_GPIO_STATE_LOW;
        break;
      default:
        // should be impossible, we verified in init that all states are valid
        return status_code(STATUS_CODE_INTERNAL_ERROR);
    }

    prv_set_pin(&output_spec->address, state);
  }

  return STATUS_CODE_OK;
}
