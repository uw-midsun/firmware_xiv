#pragma once

// Receives a DataReadyEvent
// Takes data from data_store and tx each data point in a CAN message
// Requires CAN, event_queue, soft_timers, interrupts and the data store to be initialized

#include "event_queue.h"
#include "solar_boards.h"

typedef struct DataTxSettings {
  uint32_t wait_between_tx_in_millis;
  uint8_t msgs_per_tx_iteration;
  SolarMpptCount mppt_count;
} DataTxSettings;

StatusCode data_tx_init(const DataTxSettings *settings);

bool data_tx_process_event(Event *e);
