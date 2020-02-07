#include "pedal_rx.h"

#include "can.h"
#include "can_unpack.h"
#include "can_ack.h"
#include "exported_enums.h"

static StatusCode prv_handle_pedal_output(const CanMessage* msg, void* context,
                                         CanAckStatus *ack_reply) {
  PedalRxStorage* storage = context;

  uint32_t throttle_msg;
  uint32_t brake_msg;
  CAN_UNPACK_PEDAL_OUTPUT(msg, &throttle_msg, &brake_msg);
  storage->throttle = (float)(throttle_msg) / PEDAL_RX_MSG_DENOMINATOR;
  storage->brake = (float)(brake_msg) / PEDAL_RX_MSG_DENOMINATOR;

  event_raise_priority(EVENT_PRIORITY_NORMAL, storage->settings.rx_event, 0);
  storage->watchdog_timer = PEDAL_RX_WATCHDOG_TIMEOUT_COUNTER_MAX;
  // TODO(SOFT-116): Could also use a callback here instead of storing it?
  //       that'd enable correct ack if we want to ack this message ...
  // NOTE: No ACK for pedal rx ....
  return STATUS_CODE_OK;
}

static void prv_pedal_watchdog(SoftTimerId timer_id, void *context) {
  PedalRxStorage* storage = context;
  if (storage->watchdog_timer == 0) {
    storage->throttle = 0.0f;
    storage->brake = 0.0f;
    event_raise_priority(EVENT_PRIORITY_NORMAL, storage->settings.timeout_event, 0);
  }
  if (storage->watchdog_timer >= 0) {
    // if watchdog timer was triggered, then wait until
    // next recieve of data before restarting watchdog
    storage->watchdog_timer--;
  }
  soft_timer_start_millis(PEDAL_OUTPUT_WATCHDOG_PERIOD_MS, prv_pedal_watchdog,
                          storage, NULL);
}

StatusCode pedal_rx_init(PedalRxStorage* storage, PedalRxSettings* settings) {
  storage->settings = *settings;
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_handle_pedal_output, storage);
  storage->watchdog_timer = PEDAL_RX_WATCHDOG_TIMEOUT_COUNTER_MAX;
  soft_timer_start_millis(PEDAL_OUTPUT_WATCHDOG_PERIOD_MS, prv_pedal_watchdog,
                          storage, NULL);
  return STATUS_CODE_OK;
}

float pedal_rx_get_throttle_output(PedalRxStorage* storage) {
  return storage->throttle;
}

float pedal_rx_get_brake_output(PedalRxStorage* storage) {
  return storage->brake;
}
