#include "can_tx_retry_wrapper.h"
#include "can_transmit.h"
#include "log.h"

// forward declaring
static CanAckRequest s_ack_req = { 0 };

static void prv_try_tx(CanTxRetryWrapperStorage *storage);

StatusCode can_tx_retry_wrapper_init(CanTxRetryWrapperStorage *storage,
                                     CanTxRetryWrapperSettings *settings) {
  storage->retries = settings->retries;
  storage->retry_count = 0;
  storage->success_callback = NULL;
  return STATUS_CODE_OK;
}

static StatusCode prv_can_simple_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                     uint16_t num_remaining, void *context) {
  CanTxRetryWrapperStorage *storage = (CanTxRetryWrapperStorage *)context;
  if (num_remaining || status) {
    storage->retry_count++;
    if (storage->retry_count >= storage->retries) {
      event_raise(storage->retry_request.fault_event_id, storage->retry_request.fault_event_data);
      if (!storage->retry_request.retry_indefinitely) {
        storage->retry_count = 0;
        return STATUS_CODE_RESOURCE_EXHAUSTED;
      }
    }
    if (storage->retry_count < storage->retries || storage->retry_request.retry_indefinitely) {
      prv_try_tx(storage);
    }
    return STATUS_CODE_OK;
  }
  event_raise(storage->retry_request.completion_event_id,
              storage->retry_request.completion_event_data);
  if (storage->success_callback) {
    storage->success_callback(storage->success_callback_context);
  }
  storage->retry_count = 0;
  return STATUS_CODE_OK;
}

static void prv_try_tx(CanTxRetryWrapperStorage *storage) {
  s_ack_req.callback = prv_can_simple_ack;
  s_ack_req.context = storage;
  s_ack_req.expected_bitset = storage->ack_bitset;
  if (storage->tx_callback) {
    storage->tx_callback(&s_ack_req, storage->tx_callback_context);
  }
}

StatusCode can_tx_retry_send(CanTxRetryWrapperStorage *storage, CanTxRetryWrapperRequest *request) {
  if (!request->tx_callback) {
    return STATUS_CODE_INVALID_ARGS;
  }
  storage->retry_request = request->retry_request;
  storage->tx_callback = request->tx_callback;
  storage->tx_callback_context = request->tx_callback_context;
  storage->ack_bitset = request->ack_bitset;
  prv_try_tx(storage);
  return STATUS_CODE_OK;
}

StatusCode can_tx_retry_wrapper_register_success_callback(CanTxRetryWrapperStorage *storage,
                                                          CanTxRetrySuccessCallback callback,
                                                          void *context) {
  storage->success_callback = callback;
  storage->success_callback_context = context;
  return STATUS_CODE_OK;
}
