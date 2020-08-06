#pragma once

// Requires CAN and the data store to be initialized
// Recieves a DataReadyEvent
// Takes data from data_store and tx each data point in CAN message

#include "event_queue.h"

bool data_tx_process_event(Event *e);
