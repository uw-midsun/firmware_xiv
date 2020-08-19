#include "data_tx.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "data_store.h"
#include "event_queue.h"
#include "solar_events.h"

uint16_t data_value;  // used to store value of data point
bool data_is_set;     // boolean to store state of data value (set (T) or not set (F))

// Checks if every data point is set, if true then tx data point to CAN
static void prv_data_tx() {
  for (DataPoint data_point = DATA_POINT_VOLTAGE_1; data_point < NUM_DATA_POINTS; data_point++) {
    data_store_get_is_set(data_point, &data_is_set);
    if (data_is_set) {
      data_store_get(data_point, &data_value);
      CAN_TRANSMIT_SOLAR_DATA((uint32_t)data_point, (uint32_t)data_value);
    }
  }
}

// if DataReady event is recieved, will call function to tx solar data
void data_tx_process_event(Event *e) {
  if (e->id == DATA_READY_EVENT) {
    prv_data_tx();
  }
}
