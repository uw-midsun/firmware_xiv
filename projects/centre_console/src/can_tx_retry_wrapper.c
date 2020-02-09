#include "can_transmit.h"
#include "can_tx_retry_wrapper.h"
#include "log.h"

// forward declaring
static void prv_try_tx(CanTxRetryWrapperStorage *storage);

StatusCode can_tx_retry_wrapper_init(CanTxRetryWrapperStorage *storage, CanTxRetryWrapperSettings *settings) {
  storage->retries = settings->retries;
  storage->retry_count = 0;
  return STATUS_CODE_OK;
}

static StatusCode prv_can_simple_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                     uint16_t num_remaining, void *context) {
  CanTxRetryWrapperStorage *storage = (CanTxRetryWrapperStorage *) context;
  if (num_remaining || status) {
    storage->retry_count++;
    LOG_DEBUG("retry_count: %d\n", storage->retry_count);
    if (storage->retry_count >= storage->retries) {
      event_raise(storage->fault_event_id, storage->fault_event_data);
    }
    if (storage->retry_count < storage->retries || storage->retry_indefinitely) {
      prv_try_tx(storage);
    }
    return STATUS_CODE_OK;
  }
  event_raise(storage->completion_event_id, storage->completion_event_data);
  storage->retry_count = 0;
  return STATUS_CODE_OK;
}

static void prv_try_tx(CanTxRetryWrapperStorage *storage) {
  CanAckRequest ack_req = {
    .callback = prv_can_simple_ack,
    .context = storage,
    .expected_bitset = storage->ack_bitset,
  };
  if (storage->tx_callback) {
    storage->tx_callback(&ack_req, storage->tx_callback_context);
  }
}

StatusCode can_tx_retry_send(CanTxRetryWrapperStorage *storage, CanTxRetryWrapperRequest *request) {
  if (!request->tx_callback) {
    return STATUS_CODE_INVALID_ARGS;
  }
  storage->completion_event_id = request->completion_event_id;
  storage->completion_event_data = request->completion_event_data;
  storage->fault_event_id = request->fault_event_id;
  storage->fault_event_data = request->fault_event_data;
  storage->tx_callback = request->tx_callback;
  storage->tx_callback_context = request->tx_callback_context;
  storage->ack_bitset = request->ack_bitset;
  storage->retry_indefinitely = request->retry_indefinitely;
  prv_try_tx(storage);
  return STATUS_CODE_OK;
}
