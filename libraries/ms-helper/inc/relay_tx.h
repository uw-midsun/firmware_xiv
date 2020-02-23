#pragma once

// Relay TX module. Retries sending the relay message NUM_RELAY_TX_RETRIES times, in case of
// a fail, raises a fault event passed via the RelayTxRequest. When successful, raises a
// completion event.

#include "can_tx_retry_wrapper.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "retry_tx_request.h"
#include "status.h"

#define NUM_RELAY_TX_RETRIES 5

typedef struct RelayTxStorage {
  CanTxRetryWrapperStorage can_retry_wrapper_storage;
  EERelayId relay_id;
  EERelayState relay_state;
} RelayTxStorage;

StatusCode relay_tx_init(RelayTxStorage *storage);

StatusCode relay_tx_relay_state(RelayTxStorage *storage, RetryTxRequest *request, EERelayId id,
                                EERelayState state);

SystemCanDevice *test_get_acking_device_lookup(void);
