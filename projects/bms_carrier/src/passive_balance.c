#include "passive_balance.h"
#include "log.h"

StatusCode passive_balance(uint16_t *result_arr, size_t len, LtcAfeStorage *afe) {
  // Storage for max + min voltage values as well as the cell # with the max voltage.
  uint16_t cell_voltage_max = 0;
  uint16_t cell_voltage_min = 0;
  uint16_t max_voltage_cell_num = 0;

  cell_voltage_max = cell_voltage_min = result_arr[0];

  // Iterate through all cells in storage, updating values.
  for (uint8_t i = 0; i < len; i++) {
    if (result_arr[i] > cell_voltage_max) {
      cell_voltage_max = result_arr[i];
      max_voltage_cell_num = i;
    } else if (result_arr[i] < cell_voltage_min) {
      cell_voltage_min = result_arr[i];
    }
  }
  // Balance cell, pass in whether difference meets threshold.
  return ltc_afe_toggle_cell_discharge(
      afe, max_voltage_cell_num,
      (cell_voltage_max - cell_voltage_min >= PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV));
}
