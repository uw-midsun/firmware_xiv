#pragma once

// Passive balancing module for AFE cells.  Periodically checks for differences in voltages between
// cells and calls ltc_afe_toggle_cell_discharge on the cell with the highest voltage
// to mark the cell for discharge, depending on whether the difference between the highest and
// lowest cell voltages meets the threshold for discharging.

#include "status.h"

#include "cell_sense.h"
#include "ltc_afe.h"

// Min voltage difference between highest and lowest cell values for balancing to be required.
#define PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV 25

// Iterate through all cells and call ltc_afe_toggle_cell_discharge, passing in whether the
// difference in voltages meets the threshold.
StatusCode passive_balance(uint16_t *result_arr, size_t len, LtcAfeStorage *afe);
