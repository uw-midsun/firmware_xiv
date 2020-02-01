#include "status.h"

typedef enum  {
  DESTINATION_DRIVE_STATE_NEUTRAL = 0,
  DESTINATION_DRIVE_STATE_PARKING,
  DESTINATION_DRIVE_STATE_DRIVE,
  DESTINATION_DRIVE_STATE_REVERSE,
  NUM_DESTINATION_DRIVE_STATES
} DestinationDriveState;

typedef struct DriveFsmStorage {
  DestinationDriveState destination;
} DriveFsmStorage;

StatusCode drive_fsm_init(DriveFsmStorage *storage);

bool drive_fsm_process_event(DriveFsmStorage *storage);

