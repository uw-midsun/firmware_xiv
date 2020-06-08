#pragma once
// Helper functions for the LTC6811
//
// This module is mostly exposed for the FSM. Do not use functions in this module directly.
// Requires SPI, soft timers to be initialized
//
// Assumes that:
// TODO(SOFT-9): Update to handle gpio
// Requires GPIO, Interrupts and Soft Timers to be initialized
//
// Note that all units are in 100uV.
//
// This module supports AFEs with fewer than 12 cells using the |input_bitset|.
#include "ltc_afe.h"

// Initialize the LTC6811.
// |settings.cell_bitset| and |settings.aux_bitset| should be an array of bitsets where bits 0 to 11
// represent whether we should monitor the cell input for the given device.
StatusCode ltc_afe_impl_init(LtcAfeStorage *afe, const LtcAfeSettings *settings);

// Triggers a conversion. Note that we need to wait for the conversions to complete before the
// readback will be valid.
StatusCode ltc_afe_impl_trigger_cell_conv(LtcAfeStorage *afe);
StatusCode ltc_afe_impl_trigger_aux_conv(LtcAfeStorage *afe, uint8_t device_cell);

// Reads converted voltages from the AFE into the storage result arrays.
StatusCode ltc_afe_impl_read_cells(LtcAfeStorage *afe);
StatusCode ltc_afe_impl_read_aux(LtcAfeStorage *afe, uint8_t device_cell);

// Mark cell for discharging (takes effect after config is re-written)
// |cell| should be [0, LTC_AFE_MAX_CELLS)
StatusCode ltc_afe_impl_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge);
