#include "event_queue.h"

typedef struct CanTxRetryRequest {
  EventId completion_event_id;
  uint16_t completion_event_data;
  bool retry_indefinitely;
} CanTxRetryRequest;

