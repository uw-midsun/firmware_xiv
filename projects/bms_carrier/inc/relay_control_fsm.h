#pragma once

#include <stdbool.h>

#include "event_queue.h"
#include "status.h"

typedef struct RelayStorage {
  bool hv_enabled;
  bool gnd_enabled;
} RelayStorage;

StatusCode relay_control_init(RelayStorage *storage);

bool relay_process_event(Event *e);
