#include "steering_can.h"
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
  if (e.id == STEERING_INPUT_HORN_EVENT) {
    CAN_TRANSMIT_HORN(e.data);
  }

  else if (e.id == STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD ||
           STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR) {
    CAN_TRANSMIT_LIGHTS(e.id, e.data);
  }
  return STATUS_CODE_OK;
}
