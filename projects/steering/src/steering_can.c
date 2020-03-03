#include "steering_can.h"
#include <stdio.h>
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
#include "soft_timer.h"
#include "status.h"
#include "steering_digital_input.h"

StatusCode steering_can_process_event(Event e) {
  if (e.id >= NUM_STEERING_EVENTS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (e.id == STEERING_INPUT_HORN_EVENT) {
    CAN_TRANSMIT_HORN((EEHornState)e.data);
  } else if (e.id == STEERING_RADIO_PPT_EVENT) {
    // will be added
  } else if (e.id == STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD ||
             STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR) {
    CAN_TRANSMIT_LIGHTS(EE_LIGHT_TYPE_HIGH_BEAMS, e.data);
  } else if (e.id == STEERING_REGEN_BRAKE_EVENT) {
    // will be added
  } else if (e.id == STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT) {
    // will be added
  }

  return STATUS_CODE_OK;
}
