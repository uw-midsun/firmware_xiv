#pragma once

#include "can_tx_retry_wrapper.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "retry_tx_request.h"
#include "status.h"

typedef struct RelayTxStorage {
  CanTxRetryWrapperStorage can_retry_wrapper_storage;
  EERelayId relay_id;
  EERelayState relay_state;
} RelayTxStorage;

StatusCode relay_tx_init(RelayTxStorage *storage);

StatusCode relay_tx_relay_state(RelayTxStorage *storage, RetryTxRequest *request, EERelayId id,
                                EERelayState state);
