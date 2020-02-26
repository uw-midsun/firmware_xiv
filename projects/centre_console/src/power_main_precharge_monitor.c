#include "power_main_precharge_monitor.h"
#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"

static void prv_timer_cleanup(PowerMainPrechargeMonitor *storage) {
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
}

static StatusCode prv_precharge_completed_callback(const CanMessage *msg, void *context,
                                                   CanAckStatus *ack_reply) {
  PowerMainPrechargeMonitor *storage = (PowerMainPrechargeMonitor *)context;
  *ack_reply = CAN_ACK_STATUS_OK;
  uint16_t sequence = NUM_EE_POWER_MAIN_SEQUENCES;
  CAN_UNPACK_POWER_ON_MAIN_SEQUENCE(msg, &sequence);
  if (sequence == EE_POWER_MAIN_SEQUENCE_PRECHARGE_COMPLETED) {
    event_raise(POWER_MAIN_SEQUENCE_EVENT_PRECHARGE_COMPLETE, 0);
    power_main_precharge_monitor_cancel(storage);
  }
  return STATUS_CODE_OK;
}

static void prv_timeout_cb(SoftTimerId timer_id, void *context) {
  PowerMainPrechargeMonitor *storage = (PowerMainPrechargeMonitor *)context;
  event_raise(POWER_MAIN_SEQUENCE_EVENT_FAULT, EE_POWER_MAIN_SEQUENCE_PRECHARGE_COMPLETED);
  prv_timer_cleanup(storage);
}

bool power_main_precharge_monitor_cancel(PowerMainPrechargeMonitor *storage) {
  bool cancelled = soft_timer_cancel(storage->timer_id);
  prv_timer_cleanup(storage);
  return cancelled;
}

StatusCode power_main_precharge_monitor_start(PowerMainPrechargeMonitor *storage) {
  soft_timer_cancel(storage->timer_id);
  return soft_timer_start_millis(storage->timeout_ms, prv_timeout_cb, storage, &storage->timer_id);
}

StatusCode power_main_precharge_monitor_init(PowerMainPrechargeMonitor *storage,
                                             PrechargeTimeoutMs timeout_ms) {
  storage->timeout_ms = timeout_ms;
  prv_timer_cleanup(storage);
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE,
                                              prv_precharge_completed_callback, storage));
  return STATUS_CODE_OK;
}
