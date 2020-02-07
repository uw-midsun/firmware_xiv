#include "drive_rx.h"

#include "can.h"
#include "can_unpack.h"
#include "can_ack.h"


static StatusCode prv_handle_drive_state(const CanMessage* msg, void* context,
                                         CanAckStatus *ack_reply) {
  DriveRxStorage* storage = context;
  const EventId drive_event_lookup[NUM_EE_DRIVE_STATES] = {
    [EE_DRIVE_STATE_DRIVE] = storage->settings.drive_event,
    [EE_DRIVE_STATE_NEUTRAL] = storage->settings.neutral_event,
    [EE_DRIVE_STATE_REVERSE] = storage->settings.reverse_event,
  };

  CAN_UNPACK_DRIVE_STATE(msg, (uint16_t *)&storage->drive_state);
  event_raise_priority(EVENT_PRIORITY_NORMAL, drive_event_lookup[storage->drive_state], 0);
  // TODO(SOFT-116): Could also use a callback here instead of storing it?
  //       that'd enable correct ack if we want to ack this message ...
  *ack_reply = CAN_ACK_STATUS_OK;
  return STATUS_CODE_OK;
}

StatusCode drive_rx_init(DriveRxStorage* storage, DriveRxSettings* settings) {
  storage->settings = *settings;
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_STATE, prv_handle_drive_state, storage);
  return STATUS_CODE_OK;
}
