#pragma once

#include "status.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "can_tx_retry_wrapper.h"
#include "retry_tx_request.h"

typedef struct MciOutputTxStorage {
  CanTxRetryWrapperStorage can_retry_wrapper_storage;
  EEDriveOutput drive_output;
} MciOutputTxStorage;

StatusCode mci_output_init(MciOutputTxStorage *storage);

StatusCode mci_output_tx_drive_output(MciOutputTxStorage *storage, RetryTxRequest *request, EEDriveOutput drive_output);
