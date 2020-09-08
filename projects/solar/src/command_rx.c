#include "command_rx.h"

#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "log.h"
#include "relay_fsm.h"
#include "solar_events.h"
#include "status.h"

static StatusCode prv_rx_set_relay_state(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {
  // mirror the battery relay state
  uint16_t relay_mask, relay_state;
  CAN_UNPACK_SET_RELAY_STATES(msg, &relay_mask, &relay_state);

  if ((relay_mask & (1 << EE_RELAY_ID_BATTERY)) != 0) {
    if ((relay_state & (1 << EE_RELAY_ID_BATTERY)) >> EE_RELAY_ID_BATTERY == EE_RELAY_STATE_CLOSE) {
      return relay_fsm_close();
    } else {
      return relay_fsm_open();
    }
  }

  return STATUS_CODE_OK;
}

StatusCode command_rx_init(void) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, prv_rx_set_relay_state, NULL);
}
