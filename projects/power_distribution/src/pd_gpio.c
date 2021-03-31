#include "pd_gpio.h"

#include <stdint.h>

#include "log.h"
#include "output.h"
#include "pd_events.h"
#include "status.h"

static PdGpioConfig *s_config;

StatusCode pd_gpio_init(PdGpioConfig *config) {
  if (config == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_config = config;

  // catch all invalid states here for fast failure
  for (uint8_t i = 0; i < s_config->num_events; i++) {
    for (uint8_t j = 0; j < s_config->events[i].num_outputs; j++) {
      if (s_config->events[i].outputs[j].state >= NUM_PD_GPIO_STATES) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
    }
  }

  return STATUS_CODE_OK;
}

StatusCode pd_gpio_process_event(Event *e) {
  PowerDistributionGpioEvent id = e->id;

  // find the event spec or ignore if it isn't there
  PdGpioEventSpec *event_spec = NULL;
  for (uint8_t i = 0; i < s_config->num_events; i++) {
    if (s_config->events[i].event_id == id) {
      event_spec = &s_config->events[i];
      break;
    }
  }
  if (event_spec == NULL) {
    // silently ignore unspecified events - it wasn't meant for us
    return STATUS_CODE_OK;
  }

  LOG_DEBUG("acting on spec id %d\n", id);

  // act on all the event spec's outputs
  for (uint8_t i = 0; i < event_spec->num_outputs; i++) {
    PdGpioOutputSpec *output_spec = &event_spec->outputs[i];
    OutputState state;
    switch (output_spec->state) {
      case PD_GPIO_STATE_OFF:
        state = OUTPUT_STATE_OFF;
        break;
      case PD_GPIO_STATE_ON:
        state = OUTPUT_STATE_ON;
        break;
      case PD_GPIO_STATE_SAME_AS_DATA:
        state = (e->data == 0) ? OUTPUT_STATE_OFF : OUTPUT_STATE_ON;
        break;
      case PD_GPIO_STATE_OPPOSITE_TO_DATA:
        state = (e->data == 0) ? OUTPUT_STATE_ON : OUTPUT_STATE_OFF;
        break;
      default:
        // should be impossible, we verified in init that all states are valid
        return status_code(STATUS_CODE_UNREACHABLE);
    }
    StatusCode code = output_set_state(output_spec->output, state);
    if (!status_ok(code)) {
      LOG_WARN("WARNING: got status %d setting output %d to state %d via spec on event id %d\n",
               code, output_spec->output, state, id);
    }
  }

  return STATUS_CODE_OK;
}
