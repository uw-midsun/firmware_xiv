#pragma once
// Common private functions for BTS7XXX-series load switches.
// These functions should not be called outside of the BTS7200/7040 drivers.

#include "bts7xxx_common.h"

// Fault restart delay is the same across both the BTS7040 and the BTS7200
#define BTS7XXX_FAULT_RESTART_DELAY_MS 110
#define BTS7XXX_FAULT_RESTART_DELAY_US (BTS7XXX_FAULT_RESTART_DELAY_MS * 1000)

// Broad pin soft timer cb without re-enabling the pin
void bts7xxx_fault_handler_cb(SoftTimerId timer_id, void *context);

// Broad pin re-enable soft timer cb
void bts7xxx_fault_handler_enable_cb(SoftTimerId timer_id, void *context);

// Helper function to clear fault on a given pin
StatusCode bts7xxx_handle_fault_pin(Bts7xxxEnablePin *pin);
