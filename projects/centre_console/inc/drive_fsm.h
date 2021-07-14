#pragma once

// This FSM is for handling the drive states of the car, in summary, to enter the
// drive state we need to close the motor controller relays, and also tell motor
// controller interface that we're entering the drive state. To enter parking
// we need to make sure that the ebrake is applied. This module uses relay_tx,
// and also ebrake_tx to ensure these transitions happen.

// Requires CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "fsm.h"
#include "mci_output_tx.h"
#include "precharge_monitor.h"
#include "relay_tx.h"
#include "status.h"
#include "stdbool.h"

// Temporary defines to be updated when parking brake sensor added
#define TEST_PARKING_BRAKE_STATE PARKING_BRAKE_STATE_RELEASED
#define PRECHARGE_TIMEOUT_S 4

// Temporary enum to be updated when parking brake sensor added
typedef enum {
  PARKING_BRAKE_STATE_PRESSED = 0,
  PARKING_BRAKE_STATE_RELEASED,
  NUM_PARKING_BRAKE_STATES
} ParkingBrakeState;

typedef enum {
  DRIVE_STATE_NEUTRAL = 0,
  DRIVE_STATE_PARKING,
  DRIVE_STATE_DRIVE,
  DRIVE_STATE_REVERSE,
  DRIVE_STATE_TRANSITIONING,
  NUM_DRIVE_STATES
} DriveState;

typedef struct DriveFsmStorage {
  Fsm drive_fsm;
  DriveState destination;
  DriveState current_state;
  PrechargeMonitor precharge_monitor_storage;
  MciOutputTxStorage mci_output_storage;
} DriveFsmStorage;

StatusCode drive_fsm_init(DriveFsmStorage *storage);

bool drive_fsm_process_event(DriveFsmStorage *storage, Event *e);

DriveState drive_fsm_get_global_state(DriveFsmStorage *storage);
