#pragma once
// Module to abstract handling pedal output messages
#include "event_queue.h"
#include "exported_enums.h"

// Max value is 100 = 7 bits. which leaves 25 bits of precision
#define PEDAL_RX_MSG_DENOMINATOR (1 << 25)

#define PEDAL_RX_WATCHDOG_TIMEOUT_COUNTER_MAX 5
#define PEDAL_OUTPUT_WATCHDOG_PERIOD_MS 200

typedef struct PedalRxSettings {
  EventId rx_event;
  EventId timeout_event;
} PedalRxSettings;

typedef struct PedalRxStorage {
  int watchdog_timer;
  float throttle;
  float brake;
  PedalRxSettings settings;
  void *context;
} PedalRxStorage;

StatusCode pedal_rx_init(PedalRxStorage *storage, PedalRxSettings *settings);

float pedal_rx_get_throttle_output(PedalRxStorage *storage);

float pedal_rx_get_brake_output(PedalRxStorage *storage);
