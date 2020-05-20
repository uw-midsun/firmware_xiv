#include "steering_can.h"
#include "can.h"
#include "can_ack.h"
#include "can_fsm.h"
#include "can_msg.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "status.h"
#include "steering_digital_input.h"
#include "steering_events.h"

StatusCode steering_can_process_event(Event *e) {
  switch (e->id) {
    case STEERING_INPUT_HORN_EVENT:
      CAN_TRANSMIT_HORN((EEHornState)e->data);
      break;
    case STEERING_HIGH_BEAM_FORWARD_EVENT:
      CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_HIGH_BEAMS, (EELightState)e->data);
      break;
    case STEERING_HIGH_BEAM_REAR_EVENT:
      CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_LOW_BEAMS, (EELightState)e->data);
      break;
    case STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT:
      CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
      break;
    case STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL:
      CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_SIGNAL_LEFT, (EELightState)e->data);
      break;
    case STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL:
      CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_SIGNAL_RIGHT, (EELightState)e->data);
      break;
    case STEERING_CC_EVENT_INCREASE_SPEED:
      CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_INCREASE);
      break;
    case STEERING_CC_EVENT_DECREASE_SPEED:
      CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_DECREASE);
      break;
    default:
      return STATUS_CODE_OUT_OF_RANGE;
  }
  can_process_event(e);
  return STATUS_CODE_OK;
}
