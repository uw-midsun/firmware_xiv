#include "pedal_rx.h"

#include "can.h"
#include "can_ack.h"
#include "can_unpack.h"
#include "exported_enums.h"

static void prv_pedal_watchdog(SoftTimerId timer_id, void *context) {
  PedalRxStorage *storage = context;
  PedalValues *pedal_values = &storage->pedal_values;

  pedal_values->throttle = 0.0f;
  pedal_values->brake = 0.0f;

  event_raise_priority(EVENT_PRIORITY_NORMAL, storage->settings.timeout_event, 0);
}

static StatusCode prv_kick_watchdog(PedalRxStorage *storage) {
  if (storage->watchdog_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(storage->watchdog_id);
    storage->watchdog_id = SOFT_TIMER_INVALID_TIMER;
  }
  status_ok_or_return(soft_timer_start_millis(PEDAL_RX_WATCHDOG_PERIOD_MS, prv_pedal_watchdog,
                                              storage, &storage->watchdog_id));
  return STATUS_CODE_OK;
}

static StatusCode prv_handle_pedal_output(const CanMessage *msg, void *context,
                                          CanAckStatus *ack_reply) {
  PedalRxStorage *storage = context;
  PedalValues *pedal_values = &storage->pedal_values;

  uint32_t throttle_msg;
  uint32_t brake_msg;
  CAN_UNPACK_PEDAL_OUTPUT(msg, &throttle_msg, &brake_msg);

  pedal_values->throttle = (float)(throttle_msg) / PEDAL_RX_MSG_DENOMINATOR;
  pedal_values->brake = (float)(brake_msg) / PEDAL_RX_MSG_DENOMINATOR;
  prv_kick_watchdog(storage);
  return STATUS_CODE_OK;
}

StatusCode pedal_rx_init(PedalRxStorage *storage, PedalRxSettings *settings) {
  storage->settings = *settings;
  storage->watchdog_id = SOFT_TIMER_INVALID_TIMER;
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_handle_pedal_output, storage));
  status_ok_or_return(prv_kick_watchdog(storage));
  return STATUS_CODE_OK;
}

PedalValues pedal_rx_get_pedal_values(PedalRxStorage *storage) {
  return storage->pedal_values;
}
