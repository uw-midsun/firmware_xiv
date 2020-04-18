#pragma once

// This is the module that handles the generation of events that kick off
// the power and drive state machines.
// requires event_queue, charging manager, power fsm, speed monitor, drive fsm, 
// and pedal monitor to be initialized.

// Inputs to this module are button press events.
// Generation of events depends on the following states:
//   charging state
//   power state
//   speed state
//   drive state
//   pedal state

#include "drive_fsm.h"
#include "event_queue.h"
#include "power_fsm.h"
#include "status.h"
#include "stdbool.h"

typedef struct MainEventGeneratorResources {
  PowerFsmStorage *power_fsm;
  DriveFsmStorage *drive_fsm;
} MainEventGeneratorResources;

typedef struct MainEventGeneratorStorage {
  PowerFsmStorage *power_fsm;
  DriveFsmStorage *drive_fsm;
} MainEventGeneratorStorage;

StatusCode main_event_generator_init(MainEventGeneratorStorage *storage,
                                     MainEventGeneratorResources *resources);

bool main_event_generator_process_event(MainEventGeneratorStorage *storage, const Event *event);
