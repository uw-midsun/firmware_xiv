#include "can_handler.h"

#include "bms.h"
#include "can_transmit.h"
#include "delay.h"
#include "log.h"
#include "soft_timer.h"

#define TIME_BETWEEN_TX_IN_MILLIS 100
#define NUM_AGGREGATE_VC_MSGS 1
#define NUM_BATTERY_VT_MSGS NUM_TOTAL_CELLS
#define NUM_BATTERY_RELAY_STATE_MSGS 1

static uint8_t s_msgs_txed = 0;

static void prv_periodic_tx(SoftTimerId timer_id, void *context);

static void prv_current_tx(BmsStorage *storage) {
  uint32_t avg_voltage = 0;
  for (uint8_t cell = 0; cell < NUM_TOTAL_CELLS; cell++) {
    avg_voltage += storage->afe_readings.voltages[cell];
  }
  avg_voltage /= NUM_TOTAL_CELLS;
  CAN_TRANSMIT_BATTERY_AGGREGATE_VC(avg_voltage, (uint32_t)storage->current_storage.average);
  s_msgs_txed++;
  LOG_DEBUG("did a current tx \n");
  soft_timer_start_millis(TIME_BETWEEN_TX_IN_MILLIS, prv_periodic_tx, storage, NULL);
}

static void prv_cell_voltage_and_temp_tx(BmsStorage *storage) {
  uint16_t cell_temp = storage->afe_readings.temps[s_msgs_txed * 2] >
                               storage->afe_readings.temps[s_msgs_txed * 2 + 1]
                           ? storage->afe_readings.temps[s_msgs_txed * 2]
                           : storage->afe_readings.temps[s_msgs_txed * 2 + 1];
  CAN_TRANSMIT_BATTERY_VT(s_msgs_txed, storage->afe_readings.voltages[s_msgs_txed], cell_temp);
  s_msgs_txed++;
  LOG_DEBUG("did a VT tx \n");
  soft_timer_start_millis(TIME_BETWEEN_TX_IN_MILLIS, prv_periodic_tx, storage, NULL);
}

static void prv_relay_state_tx(BmsStorage *storage) {
  CAN_TRANSMIT_BATTERY_RELAY_STATE(storage->relay_storage.hv_enabled,
                                   storage->relay_storage.gnd_enabled);
  s_msgs_txed++;
  LOG_DEBUG("did a relay tx \n");
  soft_timer_start_millis(TIME_BETWEEN_TX_IN_MILLIS, prv_periodic_tx, storage, NULL);
}
/*
static void prv_fan_status_tx(BmsStorage *storage) {
    if(storage->fan_storage.status != STATUS_CODE_OK) {
        CAN_TRANSMIT_BATTERY_FAN_STATE();
    }
}
*/

static void prv_periodic_tx(SoftTimerId timer_id, void *context) {
  BmsStorage *storage = context;

  if (s_msgs_txed < NUM_BATTERY_VT_MSGS) {
    prv_cell_voltage_and_temp_tx(storage);
  } else if (s_msgs_txed < NUM_BATTERY_VT_MSGS + NUM_AGGREGATE_VC_MSGS) {
    prv_current_tx(storage);
  } else if (s_msgs_txed <
             NUM_BATTERY_VT_MSGS + NUM_AGGREGATE_VC_MSGS + NUM_BATTERY_RELAY_STATE_MSGS) {
    prv_relay_state_tx(storage);
  }
  // prv_fan_status_tx(storage);
}

StatusCode can_handler_init(BmsStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  LOG_DEBUG("let it rip \n");
  prv_periodic_tx(SOFT_TIMER_INVALID_TIMER, storage);
  return STATUS_CODE_OK;
}
