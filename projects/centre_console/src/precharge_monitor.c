#include "precharge_monitor.h"
#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"

static void prv_timer_cleanup(PrechargeMonitor *storage) {
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
}

static StatusCode prv_precharge_completed_callback(const CanMessage *msg, void *context,
                                                   CanAckStatus *ack_reply) {
  PrechargeMonitor *storage = (PrechargeMonitor *)context;
  precharge_monitor_cancel(storage);
  event_raise(storage->success_event.id, storage->success_event.data);
  return STATUS_CODE_OK;
}

static void prv_timeout_cb(SoftTimerId timer_id, void *context) {
  PrechargeMonitor *storage = (PrechargeMonitor *)context;
  event_raise(storage->fail_event.id, storage->fail_event.data);
  prv_timer_cleanup(storage);
}

bool precharge_monitor_cancel(PrechargeMonitor *storage) {
  bool cancelled = soft_timer_cancel(storage->timer_id);
  prv_timer_cleanup(storage);
  return cancelled;
}

StatusCode precharge_monitor_start(PrechargeMonitor *storage) {
  soft_timer_cancel(storage->timer_id);
  return soft_timer_start_millis(storage->timeout_ms, prv_timeout_cb, storage, &storage->timer_id);
}

StatusCode precharge_monitor_init(PrechargeMonitor *storage, PrechargeTimeoutMs timeout_ms,
                                  const Event *success_event, const Event *fail_event) {
  storage->timeout_ms = timeout_ms;
  storage->success_event = *success_event;
  storage->fail_event = *fail_event;
  prv_timer_cleanup(storage);
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_PRECHARGE_COMPLETED,
                                              prv_precharge_completed_callback, storage));
  return STATUS_CODE_OK;
}
