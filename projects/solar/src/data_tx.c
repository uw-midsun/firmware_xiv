#include "data_tx.h"

#include "can_transmit.h"
#include "data_store.h"
#include "event_queue.h"
#include "solar_events.h"

// Checks if each data point is set, if true then tx data point to CAN
static void prv_data_tx(void) {
  uint32_t data_value;
  bool data_is_set;
  for (DataPoint data_point = 0; data_point < NUM_DATA_POINTS; data_point++) {
    data_store_get_is_set(data_point, &data_is_set);
    if (data_is_set) {
      data_store_get(data_point, &data_value);
      CAN_TRANSMIT_SOLAR_DATA((uint32_t)data_point, (uint32_t)data_value);
    }
  }
}

bool data_tx_process_event(Event *e) {
  if (e == NULL) {
    return false;
  } else if (e->id == DATA_READY_EVENT) {
    prv_data_tx();
    return true;
  }
  return false;
}
