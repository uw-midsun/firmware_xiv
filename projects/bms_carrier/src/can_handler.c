#include "can_handler.h"

#include "bms.h"
#include "can_transmit.h"
#include "delay.h"
#include "log.h"
#include "soft_timer.h"

static uint8_t s_msgs_txed = 0;

static void prv_periodic_tx(SoftTimerId timer_id, void *context);

// Calculate avg cell voltage and tx along with avg current
static void prv_current_tx(BmsStorage *storage) {
  uint32_t avg_voltage = 0;
  for (uint8_t cell = 0; cell < NUM_TOTAL_CELLS; cell++) {
    avg_voltage += storage->afe_readings.voltages[cell];
  }
  avg_voltage /= NUM_TOTAL_CELLS;
  CAN_TRANSMIT_BATTERY_AGGREGATE_VC(avg_voltage, (uint32_t)storage->current_storage.average);
  s_msgs_txed++;
  soft_timer_start_millis(TIME_BETWEEN_TX_IN_MILLIS, prv_periodic_tx, storage, NULL);
}

// Tx cell voltage and its higher temperature reading
static void prv_cell_voltage_and_temp_tx(BmsStorage *storage) {
  uint16_t cell_temp = storage->afe_readings.temps[s_msgs_txed * 2] >
                               storage->afe_readings.temps[s_msgs_txed * 2 + 1]
                           ? storage->afe_readings.temps[s_msgs_txed * 2]
                           : storage->afe_readings.temps[s_msgs_txed * 2 + 1];
  CAN_TRANSMIT_BATTERY_VT(s_msgs_txed, storage->afe_readings.voltages[s_msgs_txed], cell_temp);
  s_msgs_txed++;
  soft_timer_start_millis(TIME_BETWEEN_TX_IN_MILLIS, prv_periodic_tx, storage, NULL);
}

static void prv_relay_state_tx(BmsStorage *storage) {
  CAN_TRANSMIT_BATTERY_RELAY_STATE(storage->relay_storage.hv_enabled,
                                   storage->relay_storage.gnd_enabled);
  s_msgs_txed++;
  soft_timer_start_millis(TIME_BETWEEN_TX_IN_MILLIS, prv_periodic_tx, storage, NULL);
}

static void prv_fan_status_tx(BmsStorage *storage) {
  CAN_TRANSMIT_BATTERY_FAN_STATE(
      storage->fan_storage.statuses[0], storage->fan_storage.statuses[1],
      storage->fan_storage.statuses[2], storage->fan_storage.statuses[3],
      storage->fan_storage_1.statuses[0], storage->fan_storage_1.statuses[1],
      storage->fan_storage_1.statuses[2], storage->fan_storage_1.statuses[3]);
  s_msgs_txed++;
}

// Periodically call appropriate tx function based on which messages have already been txed
static void prv_periodic_tx(SoftTimerId timer_id, void *context) {
  BmsStorage *storage = context;
  if (s_msgs_txed >= NUM_TOTAL_MESSAGES) {
    s_msgs_txed = 0;
  }

  if (s_msgs_txed < NUM_BATTERY_VT_MSGS) {
    prv_cell_voltage_and_temp_tx(storage);
  } else if (s_msgs_txed < NUM_BATTERY_VT_MSGS + NUM_AGGREGATE_VC_MSGS) {
    prv_current_tx(storage);
  } else if (s_msgs_txed <
             NUM_BATTERY_VT_MSGS + NUM_AGGREGATE_VC_MSGS + NUM_BATTERY_RELAY_STATE_MSGS) {
    prv_relay_state_tx(storage);
  } else if (s_msgs_txed < NUM_TOTAL_MESSAGES) {
    prv_fan_status_tx(storage);
  }
}

StatusCode can_handler_init(BmsStorage *storage) {
  if (storage == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  prv_periodic_tx(SOFT_TIMER_INVALID_TIMER, storage);
  return STATUS_CODE_OK;
}
