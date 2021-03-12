#include "bps_watcher.h"
#include "event_queue.h"
#include "pd_events.h"

StatusCode prv_bps_watcher_callback_handler(const CanMessage *msg, void *context,
                                                   CanAckStatus *ack_reply) {
  uint8_t data = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &data);
  if (data)
    event_raise(POWER_DISTRIBUTION_STROBE_EVENT, 1);
  return STATUS_CODE_OK;
}

StatusCode bps_watcher_init() {
  can_register_rx_handler(SYSTEM_CAN_DEVICE_BMS_CARRIER, prv_bps_watcher_callback_handler,
                          NULL);
  return STATUS_CODE_OK;
}
