#pragma once
// Module to abstract handling drive output messages
#include "exported_enums.h"
#include "event_queue.h"
#include "status.h"

typedef struct DriveRxSettings {
  EventId drive_event;
  EventId reverse_event;
  EventId neutral_event;
} DriveRxSettings;

typedef struct DriveRxStorage {
  EEDriveState drive_state;
  DriveRxSettings settings;
  // fault_event?
  void *context;
} DriveRxStorage;

StatusCode drive_rx_init(DriveRxStorage* storage, DriveRxSettings* settings);
