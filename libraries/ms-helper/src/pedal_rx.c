#include "pedal_rx.h"

#include "can.h"
#include "can_ack.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "log.h"

static void prv_pedal_watchdog(SoftTimerId timer_id, void *context) {
  PedalRxStorage *storage = context;
  PedalValues *pedal_values = &storage->pedal_values;
  storage->watchdog_id = SOFT_TIMER_INVALID_TIMER;
  pedal_values->throttle = 0.0f;
  pedal_values->brake = 0.0f;
  event_raise(storage->timeout_event, 0);
}

static StatusCode prv_kick_watchdog(PedalRxStorage *storage) {
  if (storage->watchdog_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(storage->watchdog_id);
    storage->watchdog_id = SOFT_TIMER_INVALID_TIMER;
  }
  status_ok_or_return(soft_timer_start_millis(storage->timeout_ms, prv_pedal_watchdog, storage,
                                              &storage->watchdog_id));
  return STATUS_CODE_OK;
}

static StatusCode prv_handle_pedal_output(const CanMessage *msg, void *context,
                                          CanAckStatus *ack_reply) {
  LOG_DEBUG("GOT PEDAL OUTPUT!!!\n");
  PedalRxStorage *storage = context;
  PedalValues *pedal_values = &storage->pedal_values;

  uint32_t throttle_msg;
  uint32_t brake_msg;
  CAN_UNPACK_PEDAL_OUTPUT(msg, &throttle_msg, &brake_msg);
  LOG_DEBUG("throttle: %d brake: %d\n", (int)throttle_msg, (int)brake_msg);

  pedal_values->throttle = (float)(throttle_msg) / EE_PEDAL_VALUE_DENOMINATOR;
  pedal_values->brake = (float)(brake_msg) / EE_PEDAL_VALUE_DENOMINATOR;
  prv_kick_watchdog(storage);
  return STATUS_CODE_OK;
}

StatusCode pedal_rx_init(PedalRxStorage *storage, PedalRxSettings *settings) {
  storage->timeout_event = settings->timeout_event;
  storage->timeout_ms = settings->timeout_ms;
  storage->watchdog_id = SOFT_TIMER_INVALID_TIMER;
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_handle_pedal_output, storage));
  status_ok_or_return(prv_kick_watchdog(storage));
  return STATUS_CODE_OK;
}

PedalValues pedal_rx_get_pedal_values(PedalRxStorage *storage) {
  return storage->pedal_values;
}
