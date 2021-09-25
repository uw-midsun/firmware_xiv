#include "power_off_sequence.h"

#include "can_transmit.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "exported_enums.h"
#include "fsm.h"
#include "log.h"
#include "relay_tx.h"

FSM_DECLARE_STATE(state_none);
FSM_DECLARE_STATE(state_fault);
FSM_DECLARE_STATE(state_discharge_precharge);
FSM_DECLARE_STATE(state_turn_off_everything);
FSM_DECLARE_STATE(state_open_battery_relays);
FSM_DECLARE_STATE(state_power_off_complete);

FSM_STATE_TRANSITION(state_none) {
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_BEGIN, state_discharge_precharge);
}

FSM_STATE_TRANSITION(state_discharge_precharge) {
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_DISCHARGE_COMPLETED, state_turn_off_everything);
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_turn_off_everything) {
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_TURNED_OFF_EVERYTHING, state_open_battery_relays);
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_open_battery_relays) {
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_BATTERY_RELAYS_OPENED, state_power_off_complete);
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_power_off_complete) {
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_COMPLETE, state_none);
}

FSM_STATE_TRANSITION(state_fault) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, state_none);
}

static void prv_state_discharge(Fsm *fsm, const Event *e, void *context) {
  PowerOffSequenceStorage *storage = (PowerOffSequenceStorage *)context;
  CAN_TRANSMIT_DISCHARGE_PRECHARGE();
  LOG_DEBUG("requesting discharge\n");
  event_raise(POWER_OFF_SEQUENCE_EVENT_DISCHARGE_COMPLETED, 0);
}

static StatusCode prv_can_ack_everything_turned_off(CanMessageId msg_id, uint16_t device,
                                                    CanAckStatus status, uint16_t num_remaining,
                                                    void *context) {
  if (status == CAN_ACK_STATUS_TIMEOUT) {
    LOG_DEBUG("ack timed out\n");
  }
  if (status != CAN_ACK_STATUS_OK) {
    LOG_DEBUG("ack not ok, faulting\n");
    return event_raise(POWER_OFF_SEQUENCE_EVENT_FAULT, EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING);
  }
  if (num_remaining == 0 && status == CAN_ACK_STATUS_OK) {
    LOG_DEBUG("ack ok, raising turned off\n");
    return event_raise(POWER_OFF_SEQUENCE_EVENT_TURNED_OFF_EVERYTHING, 0);
  }
  return STATUS_CODE_OK;
}

static void prv_state_turn_off_everything(Fsm *fsm, const Event *e, void *context) {
  CanAckRequest ack_req = { .callback = prv_can_ack_everything_turned_off,
                            .expected_bitset = CAN_ACK_EXPECTED_DEVICES(
                                SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT,
                                SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR) };
  LOG_DEBUG("requesting to turn off everything\n");
  CAN_TRANSMIT_POWER_OFF_SEQUENCE(&ack_req, EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING);
}

static void prv_open_battery_relays(Fsm *fsm, const Event *e, void *context) {
  PowerOffSequenceStorage *storage = (PowerOffSequenceStorage *)context;
  RetryTxRequest req = { .completion_event_id = POWER_OFF_SEQUENCE_EVENT_BATTERY_RELAYS_OPENED,
                         .fault_event_id = POWER_OFF_SEQUENCE_EVENT_FAULT,
                         .fault_event_data = EE_POWER_OFF_SEQUENCE_OPEN_BATTERY_RELAYS,
                         .retry_indefinitely = false };
  LOG_DEBUG("requesting opening the relays\n");
  relay_tx_relay_state(&storage->relay_tx_storage, &req, EE_RELAY_ID_BATTERY, EE_RELAY_STATE_OPEN);
}

static void prv_power_off_complete(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("power off complete\n");
  event_raise(POWER_OFF_SEQUENCE_EVENT_COMPLETE, 0);
}

static void prv_fault(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("faulted\n");
  FaultReason reason = { .fields = { .area = EE_CONSOLE_FAULT_AREA_POWER_OFF, .reason = e->data } };
  event_raise(CENTRE_CONSOLE_POWER_EVENT_FAULT, reason.raw);
}

StatusCode power_off_sequence_init(PowerOffSequenceStorage *storage) {
  fsm_state_init(state_discharge_precharge, prv_state_discharge);
  fsm_state_init(state_turn_off_everything, prv_state_turn_off_everything);
  fsm_state_init(state_open_battery_relays, prv_open_battery_relays);
  fsm_state_init(state_power_off_complete, prv_power_off_complete);
  fsm_state_init(state_fault, prv_fault);
  status_ok_or_return(relay_tx_init(&storage->relay_tx_storage));
  fsm_init(&storage->sequence_fsm, "Power Off Sequence", &state_none, storage);
  return STATUS_CODE_OK;
}

bool power_off_sequence_process_event(PowerOffSequenceStorage *storage, const Event *event) {
  return fsm_process_event(&storage->sequence_fsm, event);
}
