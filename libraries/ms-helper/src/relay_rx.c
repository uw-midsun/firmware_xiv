#include "relay_rx.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "critical_section.h"
#include "gpio.h"
#include "status.h"

// CanRxHandler
static StatusCode prv_relay_rx_can_handler(const CanMessage *msg, void *context,
                                           CanAckStatus *ack_reply) {
  RelayRxStorage *storage = context;
  uint8_t state = storage->state_bound;
  // NOTE: This is a bit of a hack that exploits the fact all the relay control
  // messages are the same. The aim here is to not necessarily force a
  // constraint based on message id/name. Instead all single field u8 messages
  // will succeed in unpacking but the contents must be valid.
  CAN_UNPACK_BATTERY_RELAY_MAIN(msg, &state);
  if (state >= storage->state_bound) {
    *ack_reply = CAN_ACK_STATUS_INVALID;
  } else {
    storage->curr_state = state;
    if (!status_ok(storage->handler(storage->msg_id, storage->curr_state, storage->context))) {
      *ack_reply = CAN_ACK_STATUS_INVALID;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode relay_rx_configure_handler(RelayRxStorage *storage, SystemCanMessage msg_id,
                                      uint8_t state_bound, RelayRxHandler handler, void *context) {
  if (handler == NULL || storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  // NOTE: we explicitly don't constrain |msg_id|. In theory we could force this
  // to be a value defined for one of the relays in codegen-tooling. But in
  // theory this module could be used for any CAN controlled GPIO. Also note
  // that the storage is also extensible for this reason.

  bool disabled_in_scope = critical_section_start();
  StatusCode status = can_register_rx_handler(msg_id, prv_relay_rx_can_handler, storage);
  if (!status_ok(status)) {
    critical_section_end(disabled_in_scope);
    return status;
  }

  storage->handler = handler;
  storage->msg_id = msg_id;
  storage->curr_state = state_bound;
  storage->state_bound = state_bound;
  storage->context = context;
  critical_section_end(disabled_in_scope);
  return STATUS_CODE_OK;
}
