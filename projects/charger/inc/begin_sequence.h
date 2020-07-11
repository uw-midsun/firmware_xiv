#pragma once

// Sequence to begin charging
// Requires gpio, CAN, the event_queue, and charger_controller to be initialized.

// When the charger is plugged in, connection_sense raises an event to begin charging.
// Because all steps are synchronous, it doesn't make sense to complicate things with an FSM.
// The sequence takes the following steps:
// 1. Check charger is on
// 2. Get control pilot PWM
// 3. Turn on relay and load switch
// 4. Turn on control pilot
// 5. Activate charger

#include "charger_defs.h"
#include "event_queue.h"
#include "status.h"

StatusCode begin_sequence_init();

void begin_sequence_process_event(const Event *e);
