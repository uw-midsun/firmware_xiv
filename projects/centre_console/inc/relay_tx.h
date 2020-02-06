#pragma once

#include "status.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "can_tx_retry_wrapper.h"

typedef struct RelayTxStorage {
  uint8_t retries;
  EventId fault_event_id;
  uint16_t fault_event_data;
  EventId completion_event_id;
  uint16_t completion_event_data;
  bool retry_indefinitely;
  EERelayState state;
} RelayTxStorage;

typedef struct RelayTxRequest {
  EventId completion_event_id;
  uint16_t completion_event_data;
  EventId fault_event_id;
  uint16_t fault_event_data;
  bool retry_indefinitely;
  EERelayState state;
} RelayTxRequest;

StatusCode relay_tx_init(RelayTxStorage *storage);

StatusCode relay_tx_relay_state_and_raise_event(RelayTxStorage *storage, RelayTxRequest *request);
