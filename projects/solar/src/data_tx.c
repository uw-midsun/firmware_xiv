#include "data_tx.h"

#include "can_transmit.h"
#include "data_store.h"
#include "event_queue.h"
#include "log.h"
#include "soft_timer.h"
#include "solar_events.h"

static uint16_t s_msgs_txed;

// Checks if each data point is set, if true then tx data point to CAN
// Periodically checks and sends 8 data points until all 56 data points are covered
static void prv_data_tx(SoftTimerId timer_id, void *context) {
  uint32_t data_value;
  bool data_is_set;
  for (DataPoint data_point = s_msgs_txed; data_point < s_msgs_txed + MSG_PER_TX_ITERATION;
       data_point++) {
    data_store_get_is_set(data_point, &data_is_set);
    if (data_is_set) {
      data_store_get(data_point, &data_value);
      CAN_TRANSMIT_SOLAR_DATA((uint32_t)data_point, (uint32_t)data_value);
    }
  }
  s_msgs_txed += MSG_PER_TX_ITERATION;
  if (s_msgs_txed < NUM_DATA_POINTS)
    soft_timer_start_millis(WAIT_BEFORE_TX_IN_MILLIS, prv_data_tx, NULL, NULL);
}

bool data_tx_process_event(Event *e) {
  if (e == NULL) {
    return false;
  } else if (e->id == DATA_READY_EVENT) {
    s_msgs_txed = 0;
    soft_timer_start_millis(WAIT_BEFORE_TX_IN_MILLIS, prv_data_tx, NULL, NULL);
    return true;
  }
  return false;
}
