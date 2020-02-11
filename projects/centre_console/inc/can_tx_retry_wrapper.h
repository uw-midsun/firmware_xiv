#pragma once

#include "can_ack.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "retry_tx_request.h"

typedef void (*CanTxCallback)(CanAckRequest *ack_ptr, void *context);

typedef struct CanTxRetryWrapperStorage {
  uint8_t retries;
  uint8_t retry_count;
  RetryTxRequest retry_request;
  uint16_t ack_bitset;
  CanTxCallback tx_callback;
  void *tx_callback_context;
} CanTxRetryWrapperStorage;

typedef struct CanTxRetryWrapperSettings {
  uint8_t retries;
} CanTxRetryWrapperSettings;

typedef struct CanTxRetryWrapperRequest {
  RetryTxRequest retry_request;
  uint16_t ack_bitset;
  CanTxCallback tx_callback;
  void *tx_callback_context;
} CanTxRetryWrapperRequest;

StatusCode can_tx_retry_wrapper_init(CanTxRetryWrapperStorage *storage,
                                     CanTxRetryWrapperSettings *settings);

StatusCode can_tx_retry_send(CanTxRetryWrapperStorage *storage, CanTxRetryWrapperRequest *request);
