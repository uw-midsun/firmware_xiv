#pragma once

#include "fsm.h"
#include "relay_tx.h"

typedef struct PowerOffSequenceStorage {
  Fsm sequence_fsm;
  RelayTxStorage relay_tx_storage;
} PowerOffSequenceStorage;

StatusCode power_off_sequence_init(PowerOffSequenceStorage *storage);

bool power_off_sequence_process_event(PowerOffSequenceStorage *storage, const Event *event);
