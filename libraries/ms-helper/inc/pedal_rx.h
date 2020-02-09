#pragma once
// Module to abstract handling pedal output messages
#include "event_queue.h"
#include "exported_enums.h"
#include "soft_timer.h"

// Max value is 100 = 7 bits. which leaves 25 bits of precision
#define PEDAL_RX_MSG_DENOMINATOR (1 << 25)
#define PEDAL_RX_MAX_PEDAL_VALUE 100.0f

#define PEDAL_OUTPUT_WATCHDOG_PERIOD_MS 400

typedef struct PedalRxSettings {
  EventId rx_event;
  EventId timeout_event;
} PedalRxSettings;

typedef struct PedalValues {
  float throttle;
  float brake;
} PedalValues;

typedef struct PedalRxStorage {
  SoftTimerId watchdog_id;
  PedalValues pedal_values;
  PedalRxSettings settings;
  void *context;
} PedalRxStorage;

StatusCode pedal_rx_init(PedalRxStorage *storage, PedalRxSettings *settings);

PedalValues pedal_rx_get_pedal_values(PedalRxStorage *storage);
