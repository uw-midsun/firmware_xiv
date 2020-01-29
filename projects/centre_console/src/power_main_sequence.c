#include "power_main_sequence.h"
#include "can_ack.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "log.h"

FSM_DECLARE_STATE(state_none);
FSM_DECLARE_STATE(state_fault);
FSM_DECLARE_STATE(state_confirm_aux_status);
FSM_DECLARE_STATE(state_turn_on_driver_bms);
FSM_DECLARE_STATE(state_confirm_battery_status);
FSM_DECLARE_STATE(state_close_battery_relays);
FSM_DECLARE_STATE(state_confirm_dcdc);
FSM_DECLARE_STATE(state_turn_on_everything);
FSM_DECLARE_STATE(state_begin_precharge);
FSM_DECLARE_STATE(state_precharge_wait_till_complete);
FSM_DECLARE_STATE(state_power_main_complete);

FSM_STATE_TRANSITION(state_none) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_BEGIN, state_confirm_aux_status);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_confirm_aux_status) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_AUX_STATUS_OK, state_turn_on_driver_bms);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_turn_on_driver_bms) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_DRIVER_DISPLAY_BMS_ON, state_confirm_battery_status);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_confirm_battery_status) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_BATTERY_STATUS_OK, state_close_battery_relays);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_close_battery_relays) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_BATTERY_RELAYS_CLOSED, state_confirm_dcdc);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_confirm_dcdc) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_DC_DC_OK, state_turn_on_everything);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_turn_on_everything) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_TURNED_ON_EVERYTHING, state_begin_precharge);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_begin_precharge) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_PRECHARGE_BEGAN, state_precharge_wait_till_complete);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_precharge_wait_till_complete) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_PRECHARGE_COMPLETE, state_power_main_complete);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_power_main_complete) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_COMPLETE, state_none);
}

FSM_STATE_TRANSITION(state_fault) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT_POWER_MAIN_SEQUENCE, state_none);
}

static uint32_t s_ack_devices_lookup[NUM_EE_POWER_MAIN_SEQUENCES] = { 0 };

static void prv_init_ack_lookup(void) {
  s_ack_devices_lookup[EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS] =
      CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  s_ack_devices_lookup[EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS] =
      CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  s_ack_devices_lookup[EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS] =
      CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  s_ack_devices_lookup[EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS] =
      CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_BMS_CARRIER);
  s_ack_devices_lookup[EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC] =
      CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  s_ack_devices_lookup[EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING] = CAN_ACK_EXPECTED_DEVICES(
      SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR, SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT);
  s_ack_devices_lookup[EE_POWER_MAIN_SEQUENCE_BEGIN_PRECHARGE] =
      CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER);
}

static EventId s_next_event_lookup[NUM_EE_POWER_MAIN_SEQUENCES] = {
  [EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS] = POWER_MAIN_SEQUENCE_EVENT_AUX_STATUS_OK,
  [EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS] = POWER_MAIN_SEQUENCE_EVENT_DRIVER_DISPLAY_BMS_ON,
  [EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS] = POWER_MAIN_SEQUENCE_EVENT_BATTERY_STATUS_OK,
  [EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS] = POWER_MAIN_SEQUENCE_EVENT_BATTERY_RELAYS_CLOSED,
  [EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC] = POWER_MAIN_SEQUENCE_EVENT_DC_DC_OK,
  [EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING] = POWER_MAIN_SEQUENCE_EVENT_TURNED_ON_EVERYTHING,
  [EE_POWER_MAIN_SEQUENCE_BEGIN_PRECHARGE] = POWER_MAIN_SEQUENCE_EVENT_PRECHARGE_BEGAN,
};

static void prv_advance_sequence(PowerMainSequenceFsmStorage *storage) {
  storage->current_sequence++;
}
static uint8_t s_ack_count[NUM_EE_POWER_MAIN_SEQUENCES] = { 0 };

static void prv_init_ack_count(void) {
  for (uint8_t i = 0; i < NUM_EE_POWER_MAIN_SEQUENCES; i++) {
    s_ack_count[i] = 1;
  }
  s_ack_count[EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING] = 2;
}

static StatusCode prv_can_simple_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                     uint16_t num_remaining, void *context) {
  PowerMainSequenceFsmStorage *storage = (PowerMainSequenceFsmStorage *)context;
  EventId next_event = s_next_event_lookup[storage->current_sequence];
  s_ack_count[storage->current_sequence]--;
  if (s_ack_count[storage->current_sequence]) {
    return STATUS_CODE_OK;
  }
  if (num_remaining == 0 && status == CAN_ACK_STATUS_OK) {
    prv_advance_sequence(storage);
    return event_raise(next_event, 0);
  }
  return event_raise(POWER_MAIN_SEQUENCE_EVENT_FAULT, storage->current_sequence);
}

static void prv_populate_ack(PowerMainSequenceFsmStorage *storage, CanAckRequest *ack_req) {
  ack_req->callback = prv_can_simple_ack;
  ack_req->context = storage;
  ack_req->expected_bitset = s_ack_devices_lookup[storage->current_sequence];
}

static void prv_state_power_main_sequence_output(Fsm *fsm, const Event *e, void *context) {
  PowerMainSequenceFsmStorage *storage = (PowerMainSequenceFsmStorage *)context;
  CanAckRequest ack_req = { 0 };
  prv_populate_ack(storage, &ack_req);
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, storage->current_sequence);
}

static void prv_state_precharge_begin(Fsm *fsm, const Event *e, void *context) {
  PowerMainSequenceFsmStorage *storage = (PowerMainSequenceFsmStorage *)context;
  prv_state_power_main_sequence_output(fsm, e, context);
  power_main_precharge_monitor_start(&storage->precharge_monitor_storage);
}

static void prv_state_close_main_battery_relays_output(Fsm *fsm, const Event *e, void *context) {
  PowerMainSequenceFsmStorage *storage = (PowerMainSequenceFsmStorage *)context;
  CanAckRequest ack_req = { 0 };
  prv_populate_ack(storage, &ack_req);
  uint8_t relay_state = EE_CHARGER_SET_RELAY_STATE_CLOSE << EE_RELAY_ID_BATTERY;
  uint8_t relay_mask = 1 << EE_RELAY_ID_BATTERY;
  CAN_TRANSMIT_SET_RELAY_STATES(&ack_req, relay_mask, relay_state);
}

static void prv_state_none(Fsm *fsm, const Event *e, void *context) {
  PowerMainSequenceFsmStorage *storage = (PowerMainSequenceFsmStorage *)context;
  storage->current_sequence = EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS;
}

static void prv_state_power_main_complete(Fsm *fsm, const Event *e, void *context) {
  event_raise(POWER_MAIN_SEQUENCE_EVENT_COMPLETE, 0);
}

static void prv_state_fault(Fsm *fsm, const Event *e, void *context) {
  PowerMainSequenceFsmStorage *storage = (PowerMainSequenceFsmStorage *)context;
  power_main_precharge_monitor_cancel(&storage->precharge_monitor_storage);
  event_raise(CENTRE_CONSOLE_POWER_EVENT_FAULT_POWER_MAIN_SEQUENCE, e->data);
}

StatusCode power_main_sequence_init(PowerMainSequenceFsmStorage *storage) {
  prv_init_ack_count();
  prv_init_ack_lookup();
  power_main_precharge_monitor_init(&storage->precharge_monitor_storage);
  fsm_state_init(state_none, prv_state_none);
  fsm_state_init(state_fault, prv_state_fault);
  fsm_state_init(state_confirm_aux_status, prv_state_power_main_sequence_output);
  fsm_state_init(state_turn_on_driver_bms, prv_state_power_main_sequence_output);
  fsm_state_init(state_confirm_battery_status, prv_state_power_main_sequence_output);
  fsm_state_init(state_close_battery_relays, prv_state_close_main_battery_relays_output);
  fsm_state_init(state_confirm_dcdc, prv_state_power_main_sequence_output);
  fsm_state_init(state_turn_on_everything, prv_state_power_main_sequence_output);
  fsm_state_init(state_begin_precharge, prv_state_precharge_begin);
  fsm_state_init(state_power_main_complete, prv_state_power_main_complete);
  storage->current_sequence = EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS;
  fsm_init(&storage->sequence_fsm, "power_main_sequence_fsm", &state_none, storage);
  return STATUS_CODE_OK;
}

bool power_main_sequence_fsm_process_event(PowerMainSequenceFsmStorage *sequence_fsm,
                                           const Event *event) {
  return fsm_process_event(&sequence_fsm->sequence_fsm, event);
}
