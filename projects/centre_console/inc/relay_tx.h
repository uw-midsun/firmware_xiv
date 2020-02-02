#pragma once

#include "status.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "can_tx_retry.h"

#define RELAY_TX_RETRIES_INFINITE (~((uint8_t) 0))

typedef struct RelayTx {
  uint8_t retries;
} RelayTx;

typedef struct RelayTxRequest {
  EventId completion_event_id;
  uint16_t completion_event_data;
  bool retry_indefinitely;
  EERelayState state;
} RelayTxRequest;

StatusCode relay_tx_init();

StatusCode relay_tx_relay_state_and_raise_event(RelayTxRequest *request);
