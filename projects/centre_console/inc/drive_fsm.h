# pragma once

#include "fsm.h"
#include "stdbool.h"
#include "status.h"

typedef enum  {
  DRIVE_STATE_NEUTRAL = 0,
  DRIVE_STATE_PARKING,
  DRIVE_STATE_DRIVE,
  DRIVE_STATE_REVERSE,
  NUM_DRIVE_STATES
} DriveState;

typedef struct DriveFsmStorage {
  Fsm drive_fsm;
  DriveState destination;
} DriveFsmStorage;

StatusCode drive_fsm_init(DriveFsmStorage *storage);

bool drive_fsm_process_event(DriveFsmStorage *storage, Event *e);
