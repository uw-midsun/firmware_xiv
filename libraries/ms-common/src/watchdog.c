#include "watchdog.h"

void prv_expiry_callback(SoftTimerId timer_id, void *context) {
  WatchdogStorage *storage = (WatchdogStorage *)context;
  storage->callback(storage->callback_context);
}

void watchdog_start(WatchdogStorage *storage, WatchdogTimeout timeout_ms,
                    WatchdogExpiryCallback callback, void *context) {
  storage->timeout_ms = timeout_ms;
  storage->callback = callback;
  storage->callback_context = context;
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
  watchdog_kick(storage);
}

void watchdog_kick(WatchdogStorage *storage) {
  soft_timer_cancel(storage->timer_id);
  soft_timer_start_millis(storage->timeout_ms, prv_expiry_callback, storage, &storage->timer_id);
}

void watchdog_cancel(WatchdogStorage *storage) {
  soft_timer_cancel(storage->timer_id);
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
}
