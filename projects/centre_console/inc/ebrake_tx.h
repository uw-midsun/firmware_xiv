#pragma once

#include "status.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "can_tx_retry_wrapper.h"
#include "retry_tx_request.h"

typedef struct EbrakeTxStorage {
  CanTxRetryWrapperStorage can_retry_wrapper_storage;
  EEEbrakeState state;
} EbrakeTxStorage;

StatusCode ebrake_tx_init(EbrakeTxStorage *storage);

StatusCode ebrake_tx_brake_state(EbrakeTxStorage *storage, RetryTxRequest *request, EEEbrakeState state);
