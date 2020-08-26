#include "bms.h"
#include "exported_enums.h"
#include "relay_sequence.h"

static BmsStorage *s_storage;

StatusCode fault_bps_init(BmsStorage *storage) {
  s_storage = storage;
  return STATUS_CODE_OK;
}

// Clear fault from fault_bitmask if (clear)
// Fault BPS and open relays if (!clear)
StatusCode fault_bps(uint8_t fault_bitmask, bool clear) {
  if (clear) {
    s_storage->bps_storage.fault_bitset &= ~(fault_bitmask);
  } else {
    s_storage->bps_storage.fault_bitset |= fault_bitmask;
    relay_fault(&s_storage->relay_storage);
  }
  return STATUS_CODE_OK;
}
