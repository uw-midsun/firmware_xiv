#include "data_tx.h"

#include "can_transmit.h"
#include "data_store.h"
#include "event_queue.h"
#include "soft_timer.h"
#include "solar_events.h"

static DataTxSettings s_settings;

StatusCode data_tx_init(const DataTxSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  } else if (settings->msgs_per_tx_iteration == 0 || settings->wait_between_tx_in_millis == 0) {
    return STATUS_CODE_INVALID_ARGS;
  } else {
    s_settings = *settings;
    return STATUS_CODE_OK;
  }
}

// Checks if each data point is set, if true then tx data point to CAN
// Periodically checks and sends MSG_PER_TX_ITERATION data points until all data points are covered
static void prv_data_tx(SoftTimerId timer_id, void *context) {
  uint32_t data_value;
  bool data_is_set;
  uintptr_t msgs_txed = (uintptr_t)context;  // using this data type so the msgs_txed value can be
                                             // passed to the next iteration through *context with a
                                             // cast
  uint16_t last_tx = msgs_txed + s_settings.msgs_per_tx_iteration;
  for (DataPoint data_point = (DataPoint)msgs_txed;
       data_point < last_tx && data_point < NUM_DATA_POINTS; data_point++) {
    data_store_get_is_set(data_point, &data_is_set);
    if (data_is_set) {
      data_store_get(data_point, &data_value);
<<<<<<< HEAD
      if (CAN_TRANSMIT_SOLAR_DATA_5_MPPTS((uint32_t)data_point, data_value) ==
          STATUS_CODE_RESOURCE_EXHAUSTED) {
        break;
=======
      if (s_settings.mppt_count == SOLAR_BOARD_5_MPPTS) {
        if (CAN_TRANSMIT_SOLAR_DATA_5_MPPTS((uint32_t)data_point, data_value) ==
            STATUS_CODE_RESOURCE_EXHAUSTED) {
          break;
        }
      } else if (s_settings.mppt_count == SOLAR_BOARD_6_MPPTS) {
        if (CAN_TRANSMIT_SOLAR_DATA_6_MPPTS((uint32_t)data_point, data_value) ==
            STATUS_CODE_RESOURCE_EXHAUSTED) {
          break;
        }
>>>>>>> master
      }
    }
    msgs_txed++;
  }

  if (msgs_txed < NUM_DATA_POINTS) {
    soft_timer_start_millis(s_settings.wait_between_tx_in_millis, prv_data_tx, (void *)msgs_txed,
                            NULL);
  }
}

bool data_tx_process_event(Event *e) {
  if (e == NULL) {
    return false;
  } else if (e->id == DATA_READY_EVENT && s_settings.wait_between_tx_in_millis > 0 &&
             s_settings.msgs_per_tx_iteration > 0) {
    prv_data_tx(SOFT_TIMER_INVALID_TIMER, NULL);
    return true;
  }
  return false;
}
