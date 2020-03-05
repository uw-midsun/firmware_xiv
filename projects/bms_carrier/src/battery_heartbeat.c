#include "battery_heartbeat.h"
#include "can_ack.h"
#include "can_transmit.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Handles CAN ACK
static StatusCode prv_handle_heartbeat_ack(CanMessageId msg_id, uint16_t device,
                                           CanAckStatus status, uint16_t num_remaining,
                                           void *context) {
  BatteryHeartbeatStorage *storage = context;

  if (status != CAN_ACK_STATUS_OK) {
    storage->ack_fail_counter++;

    if (storage->ack_fail_counter >= BATTERY_HEARTBEAT_MAX_ACK_FAILS) {
      return battery_heartbeat_raise_fault(storage, EE_BATTERY_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT);
    }
  } else if (num_remaining == 0) {
    storage->ack_fail_counter = 0;
    return battery_heartbeat_clear_fault(storage, EE_BATTERY_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT);
  }

  return STATUS_CODE_OK;
}

// Handles faults
static StatusCode prv_transmit_battery_status(BatteryHeartbeatStorage *storage) {
  CanAckRequest ack_req = {
    .callback = prv_handle_heartbeat_ack,
    .context = storage,
    .expected_bitset = storage->expected_bitset,
  };

  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_req, storage->fault_bitset);

  if (storage->fault_bitset != EE_BATTERY_HEARTBEAT_STATE_OK) {
    LOG_DEBUG("Opening relay\n");
    // should open relay here
  }

  return STATUS_CODE_OK;
}

// Periodically handles state from storage
static void prv_periodic_heartbeat(SoftTimerId timer_id, void *context) {
  BatteryHeartbeatStorage *storage = context;

  prv_transmit_battery_status(storage);

  soft_timer_start_millis(storage->period_ms, prv_periodic_heartbeat, storage, NULL);
}

StatusCode battery_heartbeat_raise_fault(BatteryHeartbeatStorage *storage,
                                         EEBatteryHeartbeatFaultSource source) {
  storage->fault_bitset |= (1 << source);

  if (source == EE_BATTERY_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT) {
    return STATUS_CODE_OK;
  } else {
    return prv_transmit_battery_status(storage);
  }
}

StatusCode battery_heartbeat_clear_fault(BatteryHeartbeatStorage *storage,
                                         EEBatteryHeartbeatFaultSource source) {
  storage->fault_bitset &= ~(1 << source);

  return STATUS_CODE_OK;
}

StatusCode battery_heartbeat_init(BatteryHeartbeatStorage *storage, uint32_t period_ms,
                                  uint32_t expected_bitset) {
  storage->period_ms = period_ms;
  storage->expected_bitset = expected_bitset;

  storage->fault_bitset = 0x00;
  storage->ack_fail_counter = 0;

  return soft_timer_start_millis(storage->period_ms, prv_periodic_heartbeat, storage, NULL);
}