#pragma once

#include "status.h"
#include "stdbool.h"
#include "stdint.h"
#include "watchdog.h"


#define FAULT_MONITOR_WATCHDOG_TIMEOUT 4000

typedef enum {
  FAULT_STATUS_FAULT = 0,
  FAULT_STATUS_ACK_FAULT,
  FAULT_STATUS_OK,
  NUM_FAULT_STATUS
} FaultStatus;

FaultStatus *get_fault_status(void);

StatusCode fault_monitor_init(WatchdogTimeout timeout);
