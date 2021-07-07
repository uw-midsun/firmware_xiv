#include "steering_can.h"
#include "can.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"
#include "steering_digital_input.h"
#include "steering_events.h"

StatusCode steering_can_process_event(Event *e) {
  switch (e->id) {
    case STEERING_INPUT_HORN_EVENT:
      CAN_TRANSMIT_HORN((EEHornState)e->data);
      break;
    case STEERING_DRL_1_EVENT:
      CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_DRL, (EELightState)e->data);
      break;
    case STEERING_DRL_2_EVENT:
      CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_DRL, (EELightState)e->data);
      break;
    case STEERING_DIGITAL_INPUT_CC_TOGGLE_PRESSED_EVENT:
      CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
      break;
    case STEERING_CC_INCREASE_SPEED_EVENT:
      CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_INCREASE);
      break;
    case STEERING_CC_DECREASE_SPEED_EVENT:
      CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_DECREASE);
      break;
    case STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL:
      CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_SIGNAL_LEFT, (EELightState)e->data);
      break;
    case STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL:
      CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_SIGNAL_RIGHT, (EELightState)e->data);
      break;
    case STEERING_REGEN_BRAKE_EVENT:
      CAN_TRANSMIT_REGEN_BRAKING_TOGGLE_REQUEST();
      break;
    default:
      return STATUS_CODE_OUT_OF_RANGE;
  }
  return STATUS_CODE_OK;
}
