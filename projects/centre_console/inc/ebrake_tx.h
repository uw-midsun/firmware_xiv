#pragma once

// Module for controlling ebrake state, it uses can_tx_retry_wrapper to send ebrake state messages.
// in case of a fail, raises a fault event passed via the RelayTxRequest. When successful, raises a
// completion event.

// Requires CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "can_tx_retry_wrapper.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

#define NUM_EBRAKE_TX_RETRIES 5

typedef struct EbrakeTxStorage {
  CanTxRetryWrapperStorage can_retry_wrapper_storage;
  EEEbrakeState state;
  EEEbrakeState current_state;
} EbrakeTxStorage;

StatusCode ebrake_tx_init(EbrakeTxStorage *storage);

StatusCode ebrake_tx_brake_state(EbrakeTxStorage *storage, RetryTxRequest *request,
                                 EEEbrakeState state);

EEEbrakeState get_current_state(EbrakeTxStorage *storage);
