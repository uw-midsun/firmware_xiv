#pragma once

// This module retries a can message a number of times (provided in settings)
// and attempts to get a successful acknowledgement of that message. Message
// should be sent inside the tx_callback. The module provides the pointer to
// the acknowledgement request. If retry_indefinitely is set, the module keeps
// retrying that message.

// Despite the ack_bitset field, this module only supports messages that require
// a single ack. This is to avoid a distributed consensus problem where some
// devices successfully ack and some don't, potentially causing unintended
// side effects upon retrying message broadcast.

// Requires CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "can_ack.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"

typedef void (*CanTxCallback)(CanAckRequest *ack_ptr, void *context);
typedef void (*CanTxRetrySuccessCallback)(void *context);

typedef struct RetryTxRequest {
  EventId completion_event_id;
  uint16_t completion_event_data;
  EventId fault_event_id;
  uint16_t fault_event_data;
  bool retry_indefinitely;
} RetryTxRequest;

typedef struct CanTxRetryWrapperStorage {
  uint8_t retries;
  uint8_t retry_count;
  RetryTxRequest retry_request;
  uint16_t ack_bitset;
  CanTxCallback tx_callback;
  void *tx_callback_context;
  CanTxRetrySuccessCallback success_callback;
  void *success_callback_context;
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

StatusCode can_tx_retry_wrapper_register_success_callback(CanTxRetryWrapperStorage *storage,
                                                          CanTxRetrySuccessCallback callback,
                                                          void *context);
