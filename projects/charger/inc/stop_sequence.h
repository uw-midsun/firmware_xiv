#pragma once

// Sequence to stop charging
// Requires gpio, CAN, the event_queue, and charger_controller to be initialized.

// When the battery is full or the charger unplugged, the stop sequence will trigger.
// Because all steps are synchronous, it doesn't makes sense to complicate things with an FSM.
// The sequence steps are:
// 1. Deactivate charger
// 2. Turn off control pilot
// 3. Turn off load switch and relay

#include "event_queue.h"

void stop_sequence_process_event(const Event *e);
