#include "fsm.h"
#include "log.h"
#include "power_on_sequence_fsm.h"
#include "centre_console_events.h"
#include "exported_enums.h"
#include "can_transmit.h"
#include "event_queue.h"

FSM_DECLARE_STATE(state_none);
FSM_DECLARE_STATE(state_turn_on_driver_display);
FSM_DECLARE_STATE(state_turn_on_battery);
FSM_DECLARE_STATE(state_confirm_aux_voltage);
FSM_DECLARE_STATE(state_confirm_battery_health);
FSM_DECLARE_STATE(state_turn_on_motor_controller_interface);
FSM_DECLARE_STATE(state_close_battery_relay);
FSM_DECLARE_STATE(state_confirm_dcdc);
FSM_DECLARE_STATE(state_precharge);
FSM_DECLARE_STATE(state_close_motor_relay);
FSM_DECLARE_STATE(state_turn_on_complete);
FSM_DECLARE_STATE(state_fault);

FSM_STATE_TRANSITION(state_none) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_BEGIN, state_turn_on_driver_display);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_turn_on_driver_display) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_DRIVER_DISPLAY_IS_ON, state_turn_on_battery);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_turn_on_battery) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_BATTERY_IS_ON, state_confirm_aux_voltage);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_confirm_aux_voltage) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_AUX_VOLTAGE_OK, state_confirm_battery_health);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_confirm_battery_health) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_BATTERY_HEALTH_OK, state_turn_on_motor_controller_interface);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_turn_on_motor_controller_interface) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_MOTOR_CONTROLLER_INTERFACE_IS_ON, state_close_battery_relay);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_close_battery_relay) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_BATTERY_RELAY_CLOSED, state_confirm_dcdc);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_confirm_dcdc) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_DC_DC_OK, state_precharge);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_precharge) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_PRECHARGE_COMPLETE, state_close_motor_relay);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_close_motor_relay) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_MOTOR_RELAY_CLOSED, state_turn_on_complete);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_turn_on_complete) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_FAULT, state_fault);
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_RESET, state_none);
}

FSM_STATE_TRANSITION(state_fault) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_RESET, state_none);
}

static StatusCode prv_can_simple_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                            uint16_t num_remaining, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  CentreConsoleFaultReason fault_reason = storage->ack_fault_reason;
  EventId next_event = storage->success_ack_next_event;
  if (num_remaining == 0 && status == CAN_ACK_STATUS_OK) {
    return event_raise(next_event, 0);
  }
  return event_raise(POWER_ON_SEQUENCE_EVENT_FAULT, fault_reason);
}

static void prv_generate_simple_ack_request(CanAckRequest *request, PowerOnSequenceFsmStorage *storage, 
                    uint32_t expected_bitset,
                    CentreConsoleFaultReason reason,
                    EventId next_event) {
  storage->ack_fault_reason = reason;
  storage->success_ack_next_event = next_event;
  request->callback = prv_can_simple_ack;
  request->context = storage;
  request->expected_bitset = expected_bitset;
}

static void prv_state_turn_on_driver_display(Fsm *fsm, const Event *e, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  uint16_t output_bitset = 1 << EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY;
  uint16_t output_state = output_bitset;
  CAN_TRANSMIT_FRONT_POWER(output_bitset, output_state);
  event_raise(POWER_ON_SEQUENCE_EVENT_DRIVER_DISPLAY_IS_ON, 0);
}

static void prv_transmit_relay_message(PowerOnSequenceFsmStorage *storage, 
  SystemCanDevice acking_device,
  EERelayId relay_id,
  EventId success_event, EventId fault_event) {
  uint8_t output_bitset = 1 << relay_id;
  uint8_t output_state = output_bitset;
  CanAckRequest ack_req;
  prv_generate_simple_ack_request(&ack_req, 
    storage, 
    CAN_ACK_EXPECTED_DEVICES(acking_device),
    fault_event,
    success_event
  );
  CAN_TRANSMIT_SET_RELAY_STATES(&ack_req, output_bitset, output_state);
}

static void prv_transmit_rear_power_dist_turn_on_message(PowerOnSequenceFsmStorage *storage, 
  EERearPowerDistributionOutput output,
  EventId success_event, EventId fault_event) {
  uint16_t output_bitset = 1 << output;
  uint16_t output_state = output_bitset;
  CanAckRequest ack_req;
  prv_generate_simple_ack_request(&ack_req, storage, 
    CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR),
    fault_event,
    success_event
  );
  CAN_TRANSMIT_REAR_POWER(&ack_req, output_bitset, output_state);
}

static void prv_state_turn_on_battery(Fsm *fsm, const Event *e, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  prv_transmit_rear_power_dist_turn_on_message(
    storage,
    EE_REAR_POWER_DISTRIBUTION_OUTPUT_BMS_CARRIER,
    POWER_ON_SEQUENCE_EVENT_BATTERY_IS_ON,
    CENTRE_CONSOLE_FAULT_REASON_BMS_CARRIER_FAILED_TO_TURN_ON
  );
}

static void prv_state_confirm_aux_voltage(Fsm *fsm, const Event *e, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  CanAckRequest ack_req;
  prv_generate_simple_ack_request(&ack_req, storage, 
    CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR),
    CENTRE_CONSOLE_FAULT_REASON_AUX_VOLTAGE_LOW,
    POWER_ON_SEQUENCE_EVENT_AUX_VOLTAGE_OK
  );
  CAN_TRANSMIT_GET_AUX_STATUS(&ack_req);
}

static void prv_state_confirm_battery_health(Fsm *fsm, const Event *e, void *context) {
  event_raise(BATTERY_HEARTBEAT_EVENT_HEALTH_CHECK_REQUEST, 0);
}

static void prv_state_turn_on_motor_controller_interface(Fsm *fsm, const Event *e, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  prv_transmit_rear_power_dist_turn_on_message(
    storage,
    EE_REAR_POWER_DISTRIBUTION_OUTPUT_MCI,
    POWER_ON_SEQUENCE_EVENT_MOTOR_CONTROLLER_INTERFACE_IS_ON,
    CENTRE_CONSOLE_FAULT_REASON_MCI_FAILED_TO_TURN_ON
  );
}

static void prv_state_close_battery_relay(Fsm *fsm, const Event *e, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  prv_transmit_relay_message(storage, 
    SYSTEM_CAN_DEVICE_BMS_CARRIER,
    EE_RELAY_ID_BATTERY,
    POWER_ON_SEQUENCE_EVENT_BATTERY_RELAY_CLOSED, 
    CENTRE_CONSOLE_FAULT_REASON_BATTERY_RELAY_FAILED_TO_CLOSE
  );
}

static void prv_state_confirm_dcdc(Fsm *fsm, const Event *e, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  CanAckRequest ack_req;
  prv_generate_simple_ack_request(&ack_req, storage, 
    CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR),
    CENTRE_CONSOLE_FAULT_REASON_DCDC_NOT_SWITCHED,
    POWER_ON_SEQUENCE_EVENT_DC_DC_OK
  );
  CAN_TRANSMIT_GET_DC_DC_STATUS(&ack_req);
}

static void prv_state_precharge(Fsm *fsm, const Event *e, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  CanAckRequest ack_req;
  prv_generate_simple_ack_request(&ack_req, storage, 
    CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER),
    CENTRE_CONSOLE_FAULT_REASON_PRECHARGE_NOT_INITIATED,
    POWER_ON_SEQUENCE_EVENT_NO_OP
  );
  CAN_TRANSMIT_START_PRECHARGE(&ack_req);
}

static void prv_state_close_motor_relay(Fsm *fsm, const Event *e, void *context) {
  PowerOnSequenceFsmStorage *storage = (PowerOnSequenceFsmStorage *) context;
  uint8_t relay_bitset = 1 << EE_RELAY_ID_MOTOR_CONTROLLER;
  uint8_t relay_state = relay_bitset;
  CanAckRequest ack_req;
  prv_generate_simple_ack_request(&ack_req, storage, 
    CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER),
    CENTRE_CONSOLE_FAULT_REASON_MOTOR_RELAY_FAILED_TO_CLOSE,
    POWER_ON_SEQUENCE_EVENT_MOTOR_RELAY_CLOSED
  );
  CAN_TRANSMIT_SET_RELAY_STATES(&ack_req, relay_bitset, relay_state);
}

static void prv_state_turn_on_complete(Fsm *fsm, const Event *e, void *context) {
  event_raise(POWER_ON_SEQUENCE_EVENT_COMPLETE, 0);
  event_raise(POWER_ON_SEQUENCE_EVENT_RESET, 0);
}

StatusCode power_on_sequence_fsm_init(PowerOnSequenceFsmStorage *storage) {
  fsm_state_init(state_turn_on_driver_display, prv_state_turn_on_driver_display);
  fsm_state_init(state_turn_on_battery, prv_state_turn_on_battery);
  fsm_state_init(state_confirm_aux_voltage, prv_state_confirm_aux_voltage);
  fsm_state_init(state_confirm_battery_health, prv_state_confirm_battery_health);
  fsm_state_init(state_turn_on_motor_controller_interface, prv_state_turn_on_motor_controller_interface);
  fsm_state_init(state_close_battery_relay, prv_state_close_battery_relay);
  fsm_state_init(state_confirm_dcdc, prv_state_confirm_dcdc);
  fsm_state_init(state_precharge, prv_state_precharge);
  fsm_state_init(state_close_motor_relay, prv_state_close_motor_relay);
  fsm_state_init(state_turn_on_complete, prv_state_turn_on_complete);

  fsm_init(&storage->sequence_fsm, "power_on_sequence_fsm", &state_none, storage);
  return STATUS_CODE_OK;
}

bool power_on_sequence_fsm_process_event(PowerOnSequenceFsmStorage *sequence_fsm, const Event *event) {
  return fsm_process_event(&sequence_fsm->sequence_fsm, event);
}
