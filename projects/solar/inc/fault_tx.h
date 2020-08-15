#pragma once

// TX faults over CAN when receiving fault events.
// Requires the event queue, GPIO, interrupts, soft timers, and CAN to be initialized.

#include <stdbool.h>

#include "event_queue.h"

bool fault_tx_process_event(Event *e);
