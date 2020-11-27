#include "fault_bps.h"

#include "bms.h"
#include "can.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "log.h"
#include "relay_sequence.h"

static BmsStorage *s_storage;

StatusCode prv_confirm_status_handler(const CanMessage *msg, void *context,
                                      CanAckStatus *ack_reply) {
  uint16_t step = NUM_EE_POWER_MAIN_SEQUENCES;
  CAN_UNPACK_POWER_ON_MAIN_SEQUENCE(msg, &step);
  if (step != EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS) {
    return STATUS_CODE_OK;
  }
  if (s_storage->bps_storage.fault_bitset) {
    LOG_DEBUG("invalid, fault bitstet %d\n", s_storage->bps_storage.fault_bitset);
    // *ack_reply = CAN_ACK_STATUS_INVALID;
    // return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

StatusCode fault_bps_init(BmsStorage *storage) {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_confirm_status_handler,
                          NULL);
  s_storage = storage;
  return STATUS_CODE_OK;
}

// Fault BPS and open relays
StatusCode fault_bps_set(uint8_t fault_bitmask) {
  // do nothing if it's not a new fault
  if (s_storage->bps_storage.fault_bitset & fault_bitmask) {
    return STATUS_CODE_OK;
  }
  s_storage->bps_storage.fault_bitset |= fault_bitmask;
  if (fault_bitmask != EE_BPS_STATE_FAULT_RELAY) {
    relay_fault(&s_storage->relay_storage);
  }
  return STATUS_CODE_OK;
}

// Clear fault from fault_bitmask
StatusCode fault_bps_clear(uint8_t fault_bitmask) {
  s_storage->bps_storage.fault_bitset &= ~(fault_bitmask);
  return STATUS_CODE_OK;
}
