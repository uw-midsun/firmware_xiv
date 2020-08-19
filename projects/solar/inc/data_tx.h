#pragma once

// Recieves a DataReadyEvent
// Takes data from data_store and tx each data point in CAN message

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "data_store.h"

void data_tx_process_event(Event *e);
