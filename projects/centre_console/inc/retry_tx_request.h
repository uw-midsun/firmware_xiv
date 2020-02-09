#pragma once

#include "can_tx_retry_wrapper.h"
#include "event_queue.h"
#include "status.h"

typedef struct RetryTxRequest {
  EventId completion_event_id;
  uint16_t completion_event_data;
  EventId fault_event_id;
  uint16_t fault_event_data;
  bool retry_indefinitely;
  uint64_t data;
} RetryTxRequest;
