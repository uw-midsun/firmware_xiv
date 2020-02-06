#pragma once

#include "can_ack.h"
#include "event_queue.h"
#include "can_msg_defs.h"
#include "exported_enums.h"

typedef void (*CanTxCallback)(CanAckRequest *ack_ptr, void *context);

typedef struct CanTxRetryWrapperStorage {
  uint8_t retries;
  uint8_t retry_count;
  EventId fault_event_id;
  uint16_t fault_event_data;
  EventId completion_event_id;
  uint16_t completion_event_data;
  bool retry_indefinitely;
  uint16_t ack_bitset;
  CanTxCallback tx_callback;
  void *tx_callback_context;
} CanTxRetryWrapperStorage;

typedef struct CanTxRetryWrapperSettings {
  uint8_t retries;
} CanTxRetryWrapperSettings;

typedef struct CanTxRetryWrapperRequest {
  EventId completion_event_id;
  uint16_t completion_event_data;
  EventId fault_event_id;
  uint16_t fault_event_data;
  uint16_t ack_bitset;
  bool retry_indefinitely;
  CanTxCallback tx_callback;
  void *tx_callback_context;
} CanTxRetryWrapperRequest;


StatusCode can_tx_retry_wrapper_init(CanTxRetryWrapperStorage *storage, CanTxRetryWrapperSettings *settings);

StatusCode can_tx_retry_send(CanTxRetryWrapperStorage *storage, CanTxRetryWrapperRequest *request);
