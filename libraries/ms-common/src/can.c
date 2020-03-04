// Basically, this module glues all the components of CAN together.
//
// It hooks into the CAN HW callbacks:
// - TX ready: Used to re-raise potentially discarded TX events. We assume that
// if there are
//             elements in the TX FIFO, we have a backlog that has resulted in
//             discarded events.
// - Message RX: When the message RX callback runs, we just push the message
// into a queue and
//               raise an event. When that event is processed in the main loop
//               (can_fsm_process_event), we find the associated callback and
//               run it. In the case of an ACK, we update the associated pending
//               ACK request.
// - Bus error: In case of a bus error, we set a timer and wait to see if the
// bus has recovered
//              after the timeout. If it's still down, we raise an event.
#include "can.h"
#include <string.h>
#include "can_fsm.h"
#include "can_hw.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_BUS_OFF_RECOVERY_TIME_MS 500

// Attempts to transmit the specified message using the HW TX, overwriting the
// source device.
StatusCode prv_transmit(const CanMessage *msg);

// Handler for CAN HW TX ready events
// Re-raises potentially discarded TX events
void prv_tx_handler(void *context);

// Handler for CAN HW messaged RX events
// Dumps received messages to the RX queue and raises an event for the messages
// to be processed.
void prv_rx_handler(void *context);

// Bus error timer callback
// Checks if the bus has recovered, raising the fault event if still off
void prv_bus_error_timeout_handler(SoftTimerId timer_id, void *context);

// Handler for CAN HW bus error events
// Starts a timer to check for bus recovery
void prv_bus_error_handler(void *context);

static CanStorage *s_can_storage;

StatusCode can_init(CanStorage *storage, const CanSettings *settings) {
  if (settings->device_id >= CAN_MSG_MAX_DEVICES) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid device ID");
  }

  memset(storage, 0, sizeof(*storage));
  storage->rx_event = settings->rx_event;
  storage->tx_event = settings->tx_event;
  storage->fault_event = settings->fault_event;
  storage->device_id = settings->device_id;

  s_can_storage = storage;

  status_ok_or_return(can_fsm_init(&s_can_storage->fsm, s_can_storage));
  status_ok_or_return(can_fifo_init(&s_can_storage->tx_fifo));
  status_ok_or_return(can_fifo_init(&s_can_storage->rx_fifo));
  status_ok_or_return(can_ack_init(&s_can_storage->ack_requests));
  status_ok_or_return(can_rx_init(&s_can_storage->rx_handlers, s_can_storage->rx_handler_storage,
                                  SIZEOF_ARRAY(s_can_storage->rx_handler_storage)));

  CanHwSettings can_hw_settings = {
    .bitrate = settings->bitrate,
    .loopback = settings->loopback,
    .tx = settings->tx,
    .rx = settings->rx,
  };
  status_ok_or_return(can_hw_init(&can_hw_settings));

  can_hw_register_callback(CAN_HW_EVENT_TX_READY, prv_tx_handler, s_can_storage);
  can_hw_register_callback(CAN_HW_EVENT_MSG_RX, prv_rx_handler, s_can_storage);
  can_hw_register_callback(CAN_HW_EVENT_BUS_ERROR, prv_bus_error_handler, s_can_storage);

  return STATUS_CODE_OK;
}

StatusCode can_add_filter(CanMessageId msg_id) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  }

  CanId can_id = { .msg_id = msg_id };
  CanId mask = { 0 };
  mask.msg_id = ~mask.msg_id;

  return can_hw_add_filter(can_id.raw, mask.raw, false);
}

StatusCode can_register_rx_default_handler(CanRxHandlerCb handler, void *context) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  return can_rx_register_default_handler(&s_can_storage->rx_handlers, handler, context);
}

StatusCode can_register_rx_handler(CanMessageId msg_id, CanRxHandlerCb handler, void *context) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  return can_rx_register_handler(&s_can_storage->rx_handlers, msg_id, handler, context);
}

StatusCode can_transmit(const CanMessage *msg, const CanAckRequest *ack_request) {
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg->msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  }

  if (ack_request != NULL) {
    if (!CAN_MSG_IS_CRITICAL(msg)) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: ACK requested for non-critical message");
    }

    StatusCode ret = can_ack_add_request(&s_can_storage->ack_requests, msg->msg_id, ack_request);
    status_ok_or_return(ret);
  }

  // Basically, the idea is that all the TX and RX should be happening in the
  // main event loop. We raise an event just to ensure that the CAN TX is
  // postponed until the main event loop.
  event_raise(s_can_storage->tx_event, 1);

  return can_fifo_push(&s_can_storage->tx_fifo, msg);
}

bool can_process_event(const Event *e) {
  if (s_can_storage == NULL) {
    LOG_WARN("CAN Storage uninitialized\n");
    return false;
  }

  return fsm_process_event(&s_can_storage->fsm, e);
}

void prv_tx_handler(void *context) {
  CanStorage *can_storage = context;
  CanMessage tx_msg;

  // If we failed to TX some messages or aren't transmitting fast enough, those
  // events were discarded. Raise a TX event to trigger a transmit attempt. We
  // only raise one event since TX ready interrupts are 1-to-1.
  if (can_fifo_size(&can_storage->tx_fifo) > 0) {
    event_raise(can_storage->tx_event, 0);
  }
}

// The RX ISR will fire once for each received message
// Each event will result in one message's processing.
void prv_rx_handler(void *context) {
  CanStorage *can_storage = context;
  uint32_t rx_id = 0;
  CanMessage rx_msg = { 0 };
  size_t counter = 0;

  bool extended = false;
  while (can_hw_receive(&rx_id, &extended, &rx_msg.data, &rx_msg.dlc)) {
    if (extended) {
      // We don't handle extended messages in the network layer
      continue;
    }
    CAN_MSG_SET_RAW_ID(&rx_msg, rx_id);

    StatusCode result = can_fifo_push(&can_storage->rx_fifo, &rx_msg);
    // TODO(ELEC-251): add error handling for FSMs
    if (result != STATUS_CODE_OK) {
      return;
    }

    event_raise(can_storage->rx_event, 1);
  }
}

void prv_bus_error_timeout_handler(SoftTimerId timer_id, void *context) {
  CanStorage *can_storage = context;

  // Note that bus errors have never been tested.
  CanHwBusStatus status = can_hw_bus_status();

  if (status == CAN_HW_BUS_STATUS_OFF) {
    event_raise(can_storage->fault_event, 0);
  }
}

void prv_bus_error_handler(void *context) {
  CanStorage *can_storage = context;

  soft_timer_start_millis(CAN_BUS_OFF_RECOVERY_TIME_MS, prv_bus_error_timeout_handler, can_storage,
                          NULL);
}
