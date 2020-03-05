#pragma once
// Module to abstract handling pedal output messages
#include "event_queue.h"
#include "exported_enums.h"
#include "soft_timer.h"

typedef uint32_t PedalTimeoutMs;

typedef struct PedalRxSettings {
  EventId timeout_event;
  PedalTimeoutMs timeout_ms;
} PedalRxSettings;

typedef struct PedalValues {
  float throttle;
  float brake;
} PedalValues;

typedef struct PedalRxStorage {
  SoftTimerId watchdog_id;
  PedalValues pedal_values;
  EventId timeout_event;
  PedalTimeoutMs timeout_ms;
} PedalRxStorage;

StatusCode pedal_rx_init(PedalRxStorage *storage, PedalRxSettings *settings);

PedalValues pedal_rx_get_pedal_values(PedalRxStorage *storage);
