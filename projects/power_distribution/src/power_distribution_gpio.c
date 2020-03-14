#include "power_distribution_gpio.h"
#include "power_distribution_events.h"

static PowerDistributionGpioConfig s_config;

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
      case POWER_DISTRIBUTION_GPIO_STATE_SAME:
        state = (e->data == 0) ? PCA9539R_GPIO_STATE_LOW : PCA9539R_GPIO_STATE_HIGH;
        break;
      case POWER_DISTRIBUTION_GPIO_STATE_OPPOSITE:
        state = (e->data == 0) ? PCA9539R_GPIO_STATE_HIGH : PCA9539R_GPIO_STATE_LOW;
        break;
      default:
        // should be impossible, we verified in init that all states are valid
        return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
    
    status_ok_or_return(pca9539r_gpio_set_state(&output_spec->address, state));
  }
  
  return STATUS_CODE_OK;
}
