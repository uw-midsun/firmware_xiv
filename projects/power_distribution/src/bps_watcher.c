#include "bps_watcher.h"

#include "event_queue.h"
#include "log.h"
#include "pd_events.h"

#define NO_FAULT 0

StatusCode prv_bps_watcher_callback_handler(const CanMessage *msg, void *context,
                                            CanAckStatus *ack_reply) {
  uint8_t data = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &data);
  if (data != NO_FAULT) {
    event_raise(PD_STROBE_EVENT, 1);
    // raising a TURN EVERYTHING AUX because we need to turn main off and switch to aux
    // in the case of BPS fault
    event_raise(PD_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_AUX, 1);
  }
  return STATUS_CODE_OK;
}

StatusCode bps_watcher_init(void) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_bps_watcher_callback_handler,
                                 NULL);
}
