#include "can_fsm.h"
#include "can.h"
#include "can_hw.h"
#include "can_rx.h"

FSM_DECLARE_STATE(can_rx_fsm_handle);
FSM_DECLARE_STATE(can_tx_fsm_handle);

FSM_STATE_TRANSITION(can_rx_fsm_handle) {
  CanStorage *can_storage = fsm->context;
  FSM_ADD_TRANSITION(can_storage->rx_event, can_rx_fsm_handle);
  FSM_ADD_TRANSITION(can_storage->tx_event, can_tx_fsm_handle);
}

FSM_STATE_TRANSITION(can_tx_fsm_handle) {
  CanStorage *can_storage = fsm->context;
  FSM_ADD_TRANSITION(can_storage->rx_event, can_rx_fsm_handle);
  FSM_ADD_TRANSITION(can_storage->tx_event, can_tx_fsm_handle);
}

static StatusCode prv_handle_data_msg(CanStorage *can_storage, const CanMessage *rx_msg) {
  CanRxHandler *handler = can_rx_get_handler(&can_storage->rx_handlers, rx_msg->msg_id);
  CanAckStatus ack_status = CAN_ACK_STATUS_OK;
  StatusCode ret = STATUS_CODE_OK;

  if (handler != NULL) {
    ret = handler->callback(rx_msg, handler->context, &ack_status);

    if (CAN_MSG_IS_CRITICAL(rx_msg)) {
      CanMessage ack = {
        .msg_id = rx_msg->msg_id,
        .type = CAN_MSG_TYPE_ACK,
        .dlc = sizeof(ack_status),
        .data = ack_status,
      };

      ret = can_transmit(&ack, NULL);
      status_ok_or_return(ret);
    }
  }

  return ret;
}

static void prv_handle_rx(Fsm *fsm, const Event *e, void *context) {
  CanStorage *can_storage = context;
  CanMessage rx_msg = { 0 };

  StatusCode result = can_fifo_pop(&can_storage->rx_fifo, &rx_msg);
  if (result != STATUS_CODE_OK) {
    // We had a mismatch between number of events and number of messages, so
    // return silently Alternatively, we could use the data value of the event.
    return;
  }

  // We currently ignore failures to handle the message.
  // If needed, we could push it back to the queue.
  switch (rx_msg.type) {
    case CAN_MSG_TYPE_ACK:
      result = can_ack_handle_msg(&can_storage->ack_requests, &rx_msg);

      break;
    case CAN_MSG_TYPE_DATA:
      result = prv_handle_data_msg(can_storage, &rx_msg);

      break;
    default:
      status_msg(STATUS_CODE_UNREACHABLE, "CAN RX: Invalid type");

      return;
      // error
      break;
  }
}

// We assume that TX events are always 1-to-1.
// We expect the TX complete interrupt to raise any discarded events.
static void prv_handle_tx(Fsm *fsm, const Event *e, void *context) {
  CanStorage *can_storage = context;
  CanMessage tx_msg = { 0 };

  StatusCode result = can_fifo_peek(&can_storage->tx_fifo, &tx_msg);
  if (result != STATUS_CODE_OK) {
    // Mismatch
    return;
  }

  CanId msg_id = {
    .source_id = can_storage->device_id,  //
    .type = tx_msg.type,                  //
    .msg_id = tx_msg.msg_id,              //
  };

  // If added to mailbox, pop message from the TX queue
  StatusCode ret = can_hw_transmit(msg_id.raw, false, tx_msg.data_u8, tx_msg.dlc);
  if (ret == STATUS_CODE_OK) {
    can_fifo_pop(&can_storage->tx_fifo, NULL);
  }
}

StatusCode can_fsm_init(Fsm *fsm, CanStorage *can_storage) {
  if (fsm == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  fsm_init(fsm, "can_fsm", &can_rx_fsm_handle, can_storage);
  fsm_state_init(can_rx_fsm_handle, prv_handle_rx);
  fsm_state_init(can_tx_fsm_handle, prv_handle_tx);

  return STATUS_CODE_OK;
}
