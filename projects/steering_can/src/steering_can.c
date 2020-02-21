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

static CanStorage s_can_storage;

StatusCode steering_can_init() {
  CanSettings can_settings = {
    .device_id = STEERING_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = STEERING_CAN_EVENT_RX,
    .tx_event = STEERING_CAN_EVENT_TX,
    .fault_event = STEERING_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };

  can_init(&s_can_storage, &can_settings);
}

StatusCode steering_can_process_event(Event e) {
  CanMessage msg = {
    .msg_id = e.id,             //
    .type = CAN_MSG_TYPE_DATA,  //
    .data = e.data,             //
    .dlc = 1,                   //
  };

  if (e.id == EE_STEERING_INPUT_HORN) {
    CAN_TRANSMIT_HORN(e.data);
  } else if (e.id == EE_STEERING_HIGH_BEAM_FORWARD || EE_STEERING_HIGH_BEAM_REAR) {
    CAN_TRANSMIT_LIGHTS(e.id, e.data);
  }
}
