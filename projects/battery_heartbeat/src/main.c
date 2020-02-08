#include "battery_heartbeat.h"
#include "can_transmit.h"
//need these for can ack requests
#include "interrupt.h" 
#include "soft_timer.h"
#include "can_ack.h"
#include "log.h" 

//good
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

static StatusCode prv_handle_state(BatteryHeartbeatStorage *storage) {
  CanAckRequest ack_req = {
    .callback - prv_handle_heartbeat_ack, 
    .context = storage, 
    .expected_bitset = storage->expected_bitset, 
  }; 

  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_req, storage->fault_bitset); 

  if (storage ->fault_bitset != BATTERY_HEARTBEAT_STATE_OK) { 
    LOG_DEBUG("Opening relay\n"); 
    //open relay here
    //
  }

  return STATUS_CODE_OK; 
//send heartbeat here
}


//good
StatusCode battery_heartbeat_raise_fault(BatteryHeartbeatStorage *storage,
                                          EEBatteryHeartbeatFaultSource source) {
  storage->fault_bitset |= (1 << source); 

  if(source = EE_BATTERY_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT) {
    return STATUS_CODE_OK; 
  }

  else { //Only immediately handle if not due to ACK timeout
    return prv_handle_state(storage); 
  }
}
//good
StatusCode battery_heartbeat_clear_fault(BatteryHeartbeatStorage *storage, 
                                          EEBatteryHeartbeatFaultSource source ) {
  storage->fault_bitset &= ~(1 << source); 

  return STATUS_CODE_OK;
}