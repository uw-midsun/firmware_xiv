#include "fault_bps.h"

#include "bms.h"
#include "exported_enums.h"
#include "relay_sequence.h"

static BmsStorage *s_storage;

StatusCode fault_bps_init(BmsStorage *storage) {
  s_storage = storage;
  return STATUS_CODE_OK;
}

// Fault BPS and open relays
StatusCode fault_bps_set(uint8_t fault_bitmask) {
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
