#include "passive_balance.h"

#include <stdbool.h>
#include "log.h"

StatusCode passive_balance(uint16_t *result_arr, size_t len, LtcAfeStorage *afe) {
  // Assumes all AFEs have an equal number of cells
  // We pick the most charged cell per AFE
  bool to_balance[LTC_AFE_MAX_CELLS_PER_DEVICE * LTC_AFE_MAX_DEVICES] = { 0 };
  uint16_t cells_per_dev = len / afe->settings.num_devices;

  for (uint8_t dev = 0; dev < afe->settings.num_devices; dev++) {
    uint16_t cell_max = 0;
    uint16_t cell_min = 0;
    for (uint16_t cell = 0; cell < cells_per_dev; cell++) {
      uint16_t idx = cell + dev * cells_per_dev;
      if (result_arr[cell_max] < result_arr[idx]) {
        cell_max = idx;
      }
      if (result_arr[cell_min] > result_arr[idx]) {
        cell_min = idx;
      }
    }
    if (result_arr[cell_max] - result_arr[cell_min] >= PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV) {
      to_balance[cell_max] = true;
    }
  }

  for (uint16_t i = 0; i < len; i++) {
    ltc_afe_toggle_cell_discharge(afe, i, to_balance[i]);
  }
}
