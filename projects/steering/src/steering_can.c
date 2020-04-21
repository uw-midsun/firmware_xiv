#include "steering_can.h"
#include <stdio.h>
#include "adc_periodic_reader.h"
#include "can.h"
#include "can_ack.h"
#include "can_fifo.h"
#include "can_fsm.h"
#include "can_hw.h"
#include "can_msg.h"
#include "can_rx.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_digital_input.h"
#include "steering_events.h"

StatusCode steering_can_process_event(Event *e) {
  if (e->id >= NUM_STEERING_EVENTS) {
    return STATUS_CODE_INVALID_ARGS;
  } else if (e->id == STEERING_INPUT_HORN_EVENT) {
    CAN_TRANSMIT_HORN((EEHornState)e->data);
  } else if (e->id == STEERING_RADIO_PPT_EVENT) {
    // transmit something
  } else if (e->id == STEERING_HIGH_BEAM_FORWARD_EVENT) {
    CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_HIGH_BEAMS, (EELightState)e->data);
  } else if (e->id == STEERING_HIGH_BEAM_REAR_EVENT) {
    CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_LOW_BEAMS, (EELightState)e->data);
  } else if (e->id == STEERING_REGEN_BRAKE_EVENT) {
    // transmit something
  } else if (e->id == STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT) {
    CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  } else if (e->id == STEERING_CONTROL_STALK_EVENT_LEFT) {
    CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_SIGNAL_LEFT, (EELightState)e->data);
  } else if (e->id == STEERING_CONTROL_STALK_EVENT_RIGHT) {
    CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_SIGNAL_RIGHT, (EELightState)e->data);
  } else if (e->id == STEERING_CC_EVENT_INCREASE_SPEED) {
    CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_INCREASE);
  } else if (e->id == STEERING_CC_EVENT_DECREASE_SPEED) {
    CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_DECREASE);
  }
  return STATUS_CODE_OK;
}
