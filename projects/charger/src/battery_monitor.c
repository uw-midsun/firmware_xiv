#include "battery_monitor.h"

#include <stdint.h>

#include "can.h"
#include "can_unpack.h"
#include "charger_defs.h"
#include "charger_events.h"
#include "event_queue.h"

StatusCode prv_battery_monitor_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  uint32_t voltage = 0;
  uint32_t current = 0;  // unused
  CAN_UNPACK_BATTERY_AGGREGATE_VC(msg, &voltage, &current);
  if (voltage >= CHARGER_BATTERY_THRESHOLD) {
    event_raise(CHARGER_CHARGE_EVENT_STOP, 0);
  }

  return STATUS_CODE_OK;
}

StatusCode battery_monitor_init() {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC, prv_battery_monitor_rx, NULL);
  return STATUS_CODE_OK;
}
