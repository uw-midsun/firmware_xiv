#include "front_power_distribution_can_rx_event_mapper.h"
#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "front_power_distribution_events.h"
#include "log.h"

static const FrontPowerDistributionSignalEvent s_light_type_to_signal_event[] = {
  [EE_LIGHT_TYPE_SIGNAL_LEFT] = FRONT_POWER_DISTRIBUTION_SIGNAL_EVENT_LEFT,
  [EE_LIGHT_TYPE_SIGNAL_RIGHT] = FRONT_POWER_DISTRIBUTION_SIGNAL_EVENT_RIGHT,
  [EE_LIGHT_TYPE_SIGNAL_HAZARD] = FRONT_POWER_DISTRIBUTION_SIGNAL_EVENT_HAZARD,
};

static const FrontPowerDistributionGpioEvent s_output_to_gpio_event[] = {
  [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY] =
      FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY,
  [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STEERING] = FRONT_POWER_DISTRIBUTION_GPIO_EVENT_STEERING,
  [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_CENTRE_CONSOLE] =
      FRONT_POWER_DISTRIBUTION_GPIO_EVENT_CENTRE_CONSOLE,
  [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRL] = FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRL,
  [EE_FRONT_POWER_DISTRIBUTION_OUTPUT_PEDAL] = FRONT_POWER_DISTRIBUTION_GPIO_EVENT_PEDAL,
};

static StatusCode prv_handle_lights_rx(const CanMessage *msg, void *context, CanAckStatus *ack) {
  EELightType light_type = msg->data_u16[0];
  EELightState light_state = msg->data_u16[1];

  if (light_type >= NUM_EE_LIGHT_TYPES) {
    LOG_WARN("Unknown light type received: %d\r\n", light_type);
    return STATUS_CODE_OUT_OF_RANGE;
  }
  if (light_state >= NUM_EE_LIGHT_STATES) {
    LOG_WARN("Unknown light state recieved: %d\r\n", light_state);
    return STATUS_CODE_OUT_OF_RANGE;
  }

  // What to do about EE_LIGHT_TYPE_HIGH_BEAMS, LOW_BEAMS, etc?

  uint16_t data = (light_state == EE_LIGHT_STATE_ON) ? 1 : 0;

  if (light_type == EE_LIGHT_TYPE_SIGNAL_LEFT || light_type == EE_LIGHT_TYPE_SIGNAL_RIGHT ||
      light_type == EE_LIGHT_TYPE_SIGNAL_HAZARD) {
    return event_raise_priority(EVENT_PRIORITY_NORMAL, s_light_type_to_signal_event[light_type],
                                data);
  }

  if (light_type == EE_LIGHT_TYPE_DRL) {
    return event_raise_priority(EVENT_PRIORITY_NORMAL, FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRL,
                                data);
  }

  return STATUS_CODE_OK;  // not meant for us?
}

static StatusCode prv_handle_front_power_rx(const CanMessage *msg, void *context,
                                            CanAckStatus *ack) {
  EEFrontPowerDistributionOutput output = msg->data_u16[0];
  EEFrontPowerDistributionOutputState output_state = msg->data_u16[1];

  if (output >= NUM_EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STATES) {
    LOG_WARN("Unknown front power distribution output received: %d\r\n", output);
    return STATUS_CODE_OUT_OF_RANGE;
  }
  if (output_state >= NUM_EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STATES) {
    LOG_WARN("Unknown front power distribution output state received: %d\r\n", output_state);
    return STATUS_CODE_OUT_OF_RANGE;
  }

  uint16_t data = (output_state == EE_FRONT_POWER_DISTRIBUTION_OUTPUT_STATE_ON) ? 1 : 0;
  return event_raise_priority(EVENT_PRIORITY_NORMAL, s_output_to_gpio_event[output], data);
}

static StatusCode prv_handle_horn_rx(const CanMessage *msg, void *context, CanAckStatus *ack) {
  EEHornState state = msg->data_u8[0];

  if (state >= NUM_EE_HORN_STATES) {
    LOG_WARN("Unknown horn state received: %d\r\n", state);
    return STATUS_CODE_OUT_OF_RANGE;
  }

  uint16_t data = (state == EE_HORN_STATE_ON) ? 1 : 0;
  return event_raise_priority(EVENT_PRIORITY_NORMAL, FRONT_POWER_DISTRIBUTION_GPIO_EVENT_HORN,
                              data);
}

StatusCode front_power_distribution_can_rx_event_mapper_init(void) {
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS, &prv_handle_lights_rx, NULL));
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS, &prv_handle_front_power_rx, NULL));
  return STATUS_CODE_OK;
}
