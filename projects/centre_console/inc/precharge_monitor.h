#pragma once

// A simple module that waits for precharge to complete and times out after a couple of seconds.
// Requires can to be initialized.

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

typedef uint32_t PrechargeTimeoutMs;

typedef struct PrechargeMonitor {
  SoftTimerId timer_id;
  PrechargeTimeoutMs timeout_ms;
  Event success_event;
  Event fail_event;
} PrechargeMonitor;

StatusCode precharge_monitor_init(PrechargeMonitor *storage, PrechargeTimeoutMs timeout_ms,
                                  const Event *success_event, const Event *fail_event);

StatusCode precharge_monitor_start(PrechargeMonitor *storage);

bool precharge_monitor_cancel(PrechargeMonitor *storage);
