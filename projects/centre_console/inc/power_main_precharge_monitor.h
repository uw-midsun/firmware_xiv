#pragma once

// A simple module that waits for precharge to complete and times out after a couple of seconds.
// Requires can to be initialized.

#include "soft_timer.h"
#include "status.h"

#define PRECHARGE_TIMEOUT_S 5

typedef struct PowerMainPrechargeMonitor {
  SoftTimerId timer_id;
} PowerMainPrechargeMonitor;

StatusCode power_main_precharge_monitor_init(PowerMainPrechargeMonitor *storage);

StatusCode power_main_precharge_monitor_start(PowerMainPrechargeMonitor *storage);

bool power_main_precharge_monitor_cancel(PowerMainPrechargeMonitor *storage);
