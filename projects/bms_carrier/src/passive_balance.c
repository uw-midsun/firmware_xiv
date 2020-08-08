#include "passive_balance.h"
#include "log.h"

// Callback for soft timer
static void prv_passive_balance_cb(SoftTimerId timer_id, void *context) {
  passive_balance(context);

  soft_timer_start_millis(PASSIVE_BALANCE_INTERVAL_MS, prv_passive_balance_cb, context, NULL);
}

// Start passive balancing
StatusCode passive_balance_init(LtcAfeStorage *storage) {
  soft_timer_start_millis(PASSIVE_BALANCE_INTERVAL_MS, prv_passive_balance_cb, storage, NULL);
  return STATUS_CODE_OK;
}

// Iterate through cells and balance highest voltage cell if required
void passive_balance(LtcAfeStorage *storage) {
  // Storage for max + min voltage values as well as the cell # with the max voltage.
  uint16_t cell_voltage_max = 0;
  uint16_t cell_voltage_min = 0;
  uint16_t max_voltage_cell_num = 0;

  // LtcAfeStorage *storage = context;

  cell_voltage_max = cell_voltage_min = storage->cell_voltages[0];

  // Iterate through all cells in storage, updating values
  for (uint8_t i = 0; i < LTC_AFE_MAX_CELLS; i++) {
    if (storage->cell_voltages[i] > cell_voltage_max) {
      cell_voltage_max = storage->cell_voltages[i];
      max_voltage_cell_num = i;
    } else if (storage->cell_voltages[i] < cell_voltage_min) {
      cell_voltage_min = storage->cell_voltages[i];
    }
  }

  // Balance cell, pass in whether difference meets threshold.
  ltc_afe_toggle_cell_discharge(
      storage, max_voltage_cell_num,
      (cell_voltage_max - cell_voltage_min >= PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV));
}
