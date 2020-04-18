#pragma once

#include "soft_timer.h"
#include "status.h"

typedef uint32_t WatchdogTimeout;
typedef void (*WatchdogExpiryCallback)(void *context);

typedef struct WatchdogStorage {
  SoftTimerId timer_id;
  WatchdogTimeout timeout_ms;
  WatchdogExpiryCallback callback;
  void *callback_context;
} WatchdogStorage;

typedef struct WatchdogSettings {
  WatchdogTimeout timeout_ms;
  WatchdogExpiryCallback callback;
  void *callback_context;
} WatchdogSettings;

void watchdog_start(WatchdogStorage *storage, WatchdogTimeout timeout_ms,
                    WatchdogExpiryCallback callback, void *context);

void watchdog_kick(WatchdogStorage *storage);
