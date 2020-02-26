#pragma once

// This module ensures that MCI fully transitions between different drive states.
// Uses can_tx_retry_wrapper to ensure proper delivery of SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT
// messages.

// Requires CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "can_tx_retry_wrapper.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

#define NUM_MCI_OUTPUT_TX_RETRIES 5

typedef struct MciOutputTxStorage {
  CanTxRetryWrapperStorage can_retry_wrapper_storage;
  EEDriveOutput drive_output;
} MciOutputTxStorage;

StatusCode mci_output_init(MciOutputTxStorage *storage);

StatusCode mci_output_tx_drive_output(MciOutputTxStorage *storage, RetryTxRequest *request,
                                      EEDriveOutput drive_output);
