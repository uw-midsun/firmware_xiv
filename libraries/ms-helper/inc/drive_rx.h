#pragma once
// Module to abstract handling drive output messages
#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

typedef struct DriveRxSettings {
  EventId drive_event;
  EventId reverse_event;
  EventId neutral_event;
  EventId parking_event;
} DriveRxSettings;

typedef struct DriveRxStorage {
  EEDriveState drive_state;
  DriveRxSettings settings;
  // fault_event?
  void *context;
} DriveRxStorage;

StatusCode drive_rx_init(DriveRxStorage *storage, DriveRxSettings *settings);

EEDriveState drive_rx_get_state(DriveRxStorage *storage);
