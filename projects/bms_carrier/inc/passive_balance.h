#pragma once

// Passive balancing module for AFE cells.  Periodically checks for differences in voltages between
// cells and raises an event in the AFE FSM to set the balance_control pin for the cell with the
// highest voltage. Only balances if the difference between the max and min voltages >= 25 mV.

// Requires soft timers to be initialized.

#include "soft_timer.h"
#include "status.h"

#include "cell_sense.h"
#include "ltc_afe.h"

// LtcAfeStorage will need to change to AfeReadings once SOFT-9 is merged in

// Interval, in ms, between checks of cell voltages.
#define PASSIVE_BALANCE_INTERVAL_MS 1000

// Min voltage difference between highest and lowest cell values for balancing to be required.
#define PASSIVE_BALANCE_MIN_VOLTAGE_DIFF_MV 25

// Initialize, start soft timers
StatusCode passive_balance_init(LtcAfeStorage *storage);

// Iterate through all cells and call balance_control if required.
void passive_balance(LtcAfeStorage *storage);
