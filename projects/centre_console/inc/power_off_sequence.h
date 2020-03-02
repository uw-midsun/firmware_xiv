#pragma once

#include "fsm.h"
#include "relay_tx.h"

typedef struct PowerOffSequenceStorage {
  Fsm sequence_fsm;
  RelayTxStorage relay_tx_storage;
} PowerOffSequenceStorage;
