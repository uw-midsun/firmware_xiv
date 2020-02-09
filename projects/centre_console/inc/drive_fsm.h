#pragma once

#include "ebrake_tx.h"
#include "fsm.h"
#include "mci_output_tx.h"
#include "relay_tx.h"
#include "status.h"
#include "stdbool.h"

typedef enum {
  DRIVE_STATE_NEUTRAL = 0,
  DRIVE_STATE_PARKING,
  DRIVE_STATE_DRIVE,
  DRIVE_STATE_REVERSE,
  NUM_DRIVE_STATES
} DriveState;

typedef struct DriveFsmStorage {
  Fsm drive_fsm;
  DriveState destination;
  RelayTxStorage relay_storage;
  EbrakeTxStorage ebrake_storage;
  MciOutputTxStorage mci_output_storage;
} DriveFsmStorage;

StatusCode drive_fsm_init(DriveFsmStorage *storage);

bool drive_fsm_process_event(DriveFsmStorage *storage, Event *e);

DriveState *drive_fsm_get_global_state(DriveFsmStorage *storage);
