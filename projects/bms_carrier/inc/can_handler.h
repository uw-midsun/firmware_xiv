#pragma once

// Once initialized, txes cell voltages, temperatures, avg current, avg voltage, relay states and
// fan states preiodically Requires CAN, event_queue, soft_timers and interrupts to be initialized

#include "status.h"

#include "bms.h"

#define TIME_BETWEEN_TX_IN_MILLIS 150
#define NUM_AGGREGATE_VC_MSGS 1
#define NUM_BATTERY_VT_MSGS NUM_TOTAL_CELLS
#define NUM_BATTERY_RELAY_STATE_MSGS 1
#define NUM_FAN_STATE_MSGS 1
#define NUM_TOTAL_MESSAGES \
  (NUM_AGGREGATE_VC_MSGS + NUM_BATTERY_VT_MSGS + NUM_BATTERY_RELAY_STATE_MSGS + NUM_FAN_STATE_MSGS)

StatusCode can_handler_init(BmsStorage *storage);
