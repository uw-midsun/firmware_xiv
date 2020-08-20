#pragma once

// Requires CAN and the data store to be initialized
// Recieves a DataReadyEvent
// Takes data from data_store and tx each data point in a CAN message

#include "event_queue.h"

#define WAIT_BEFORE_TX_IN_MILLIS 100
#define MSG_PER_TX_ITERATION 8

bool data_tx_process_event(Event *e);
