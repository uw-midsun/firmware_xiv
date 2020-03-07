#pragma once

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
