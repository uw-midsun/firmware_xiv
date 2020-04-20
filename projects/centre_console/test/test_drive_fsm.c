#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "drive_fsm.h"
#include "ebrake_tx.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "mci_output_tx.h"
#include "ms_test_helpers.h"
#include "relay_tx.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static DriveFsmStorage s_drive_fsm = { 0 };
static CanStorage s_can_storage;
static EEEbrakeState s_ebrake_state;
static EEDriveOutput s_drive_output;

static StatusCode prv_rx_ebrake_callback(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {
  uint8_t ebrake_state;
  CAN_UNPACK_SET_EBRAKE_STATE(msg, &ebrake_state);
  s_ebrake_state = ebrake_state;
  return STATUS_CODE_OK;
}

static StatusCode prv_rx_drive_output_callback(const CanMessage *msg, void *context,
                                               CanAckStatus *ack_reply) {
  uint16_t drive_output;
  CAN_UNPACK_DRIVE_OUTPUT(msg, &drive_output);
  s_drive_output = drive_output;
  return STATUS_CODE_OK;
}

void prv_assert_current_drive_state(DriveState state) {
  TEST_ASSERT_EQUAL(state, drive_fsm_get_global_state(&s_drive_fsm));
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  const CanSettings s_can_settings = { .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                       .loopback = true,
                                       .bitrate = CAN_HW_BITRATE_500KBPS,
                                       .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                       .tx = { .port = GPIO_PORT_A, .pin = 12 },
                                       .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
                                       .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
                                       .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT };
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_EBRAKE_STATE, prv_rx_ebrake_callback, NULL));
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, prv_rx_drive_output_callback, NULL));
}

static DriveFsmInputEvent s_mci_output_lookup[] = {
  [EE_DRIVE_OUTPUT_DRIVE] = DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_DRIVE,
  [EE_DRIVE_OUTPUT_REVERSE] = DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_REVERSE,
  [EE_DRIVE_OUTPUT_OFF] = DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_OFF
};

void prv_assert_ebrake_state(Event *event, EEEbrakeState state) {
  prv_assert_current_drive_state(DRIVE_STATE_TRANSITIONING);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_EBRAKE_STATE,
                                         SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT,
                                         CAN_ACK_STATUS_OK);
  Event e = { 0 };
  TEST_ASSERT_EQUAL(s_ebrake_state, state);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e,
                                   (state == EE_EBRAKE_STATE_PRESSED)
                                       ? DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED
                                       : DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED,
                                   0);
  *event = e;
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void prv_recover_from_fault(Event *event, EEEbrakeState state) {
  // Send discharge message
  MS_TEST_HELPER_CAN_TX(CENTRE_CONSOLE_EVENT_CAN_TX);
  MS_TEST_HELPER_CAN_TX(CENTRE_CONSOLE_EVENT_CAN_TX);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e,
                                   (state == EE_EBRAKE_STATE_PRESSED)
                                       ? DRIVE_FSM_INPUT_EVENT_FAULT_RECOVER_EBRAKE_PRESSED
                                       : DRIVE_FSM_INPUT_EVENT_FAULT_RECOVER_RELEASED,
                                   0);
  MS_TEST_HELPER_CAN_RX(CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_RX(CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  *event = e;
}

void prv_fail_ebrake_state(Event *event, EEEbrakeState state) {
  for (uint8_t i = 0; i < NUM_EBRAKE_TX_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
    MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_EBRAKE_STATE,
                                           SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT,
                                           CAN_ACK_STATUS_INVALID);
  }
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(
      e, DRIVE_FSM_INPUT_EVENT_FAULT,
      ((StateTransitionFault){
           .state_machine = DRIVE_FSM_STATE_MACHINE,
           .fault_reason = (DriveFsmFaultReason){ .step = DRIVE_FSM_TRANSITION_STEP_EBRAKE_STATE,
                                                  .state = state }
                               .raw })
          .raw);
  *event = e;
}

void prv_fail_mci_output(Event *event, EEDriveOutput output) {
  for (uint8_t i = 0; i < NUM_MCI_OUTPUT_TX_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
    MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT,
                                           SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
                                           CAN_ACK_STATUS_INVALID);
  }
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(
      e, DRIVE_FSM_INPUT_EVENT_FAULT,
      ((StateTransitionFault){
           .state_machine = DRIVE_FSM_STATE_MACHINE,
           .fault_reason = (DriveFsmFaultReason){ .step = DRIVE_FSM_TRANSITION_STEP_MCI_OUTPUT,
                                                  .state = output }
                               .raw })
          .raw);
  *event = e;
}

void prv_assert_mci_output(Event *event, EEDriveOutput output) {
  prv_assert_current_drive_state(DRIVE_STATE_TRANSITIONING);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT,
                                         SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, CAN_ACK_STATUS_OK);
  Event e = { 0 };
  TEST_ASSERT_EQUAL(s_drive_output, output);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, s_mci_output_lookup[output], 0);
  *event = e;
}

DriveState s_drive_state_lookup[] = {
  [DRIVE_FSM_OUTPUT_EVENT_DRIVE] = DRIVE_STATE_DRIVE,
  [DRIVE_FSM_OUTPUT_EVENT_REVERSE] = DRIVE_STATE_REVERSE,
  [DRIVE_FSM_OUTPUT_EVENT_PARKING] = DRIVE_STATE_PARKING,
  [DRIVE_FSM_OUTPUT_EVENT_NEUTRAL] = DRIVE_STATE_NEUTRAL,
};

void prv_assert_output_event(DriveFsmOutputEvent event) {
  prv_assert_current_drive_state(s_drive_state_lookup[event]);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, event, 0);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void prv_assert_set_discharge(Event *e) {
  // send discharge message
  MS_TEST_HELPER_CAN_TX(CENTRE_CONSOLE_EVENT_CAN_TX);
  Event event = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(event, DRIVE_FSM_INPUT_EVENT_DISCHARGE_COMPLETED, 0);
  MS_TEST_HELPER_CAN_RX(CENTRE_CONSOLE_EVENT_CAN_RX);
  *e = event;
}

void prv_assert_set_precharge(Event *e) {
  prv_assert_current_drive_state(DRIVE_STATE_TRANSITIONING);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  // precharge completed
  CAN_TRANSMIT_PRECHARGE_COMPLETED();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  Event event = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(event, DRIVE_FSM_INPUT_EVENT_PRECHARGE_COMPLETED, 0);
  *e = event;
}

void teardown_test(void) {}

void test_transition_to_drive_then_parking_then_reverse(void) {
  // neutral -> drive -> parking -> reverse -> drive -> reverse
  TEST_ASSERT_OK(drive_fsm_init(&s_drive_fsm));
  prv_assert_current_drive_state(DRIVE_STATE_NEUTRAL);

  Event e = { .id = DRIVE_FSM_INPUT_EVENT_DRIVE, .data = NUM_DRIVE_STATES };
  // neutral -> set precharge
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_set_precharge(&e);

  // set precharge -> set ebrake states
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));

  // ebrake states sent
  prv_assert_ebrake_state(&e, EE_EBRAKE_STATE_RELEASED);

  // set ebrake state -> neutral precharged
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_current_drive_state(DRIVE_STATE_TRANSITIONING);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_DRIVE, 0);

  // neutral precharged -> set mci state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_mci_output(&e, EE_DRIVE_OUTPUT_DRIVE);

  // set mci state -> drive
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_DRIVE);

  e.id = DRIVE_FSM_INPUT_EVENT_PARKING;

  // drive -> set mci state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_mci_output(&e, EE_DRIVE_OUTPUT_OFF);

  // set mci state -> neutral precharged
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_PARKING, 0);

  // neutral precharged -> set ebrake state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_ebrake_state(&e, EE_EBRAKE_STATE_PRESSED);

  // set ebrake state -> set precharge
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_set_discharge(&e);

  // set precharge -> parking
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_PARKING);

  e.id = DRIVE_FSM_INPUT_EVENT_REVERSE;

  // parking -> set precharge
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_set_precharge(&e);

  // set precharge -> set ebrake
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_ebrake_state(&e, EE_EBRAKE_STATE_RELEASED);

  // set ebrake state -> neutral precharged
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_REVERSE, 0);

  // neutral precharged -> set mci output
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_mci_output(&e, EE_DRIVE_OUTPUT_REVERSE);

  // set mci output -> reverse
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_REVERSE);

  // reverse -> set mci output
  e.id = DRIVE_FSM_INPUT_EVENT_DRIVE;
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_mci_output(&e, EE_DRIVE_OUTPUT_DRIVE);

  // set mci output -> drive
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_DRIVE);

  e.id = DRIVE_FSM_INPUT_EVENT_REVERSE;
  // drive -> set mci output
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_mci_output(&e, EE_DRIVE_OUTPUT_REVERSE);

  // set mci output -> reverse
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_REVERSE);
}

void test_transition_to_parking_drive_reverse_neutral_parking(void) {
  // neutral -> parking -> drive -> reverse -> neutral
  TEST_ASSERT_OK(drive_fsm_init(&s_drive_fsm));

  Event e = { .id = DRIVE_FSM_INPUT_EVENT_PARKING, .data = NUM_DRIVE_STATES };
  // neutral -> set ebrake state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_ebrake_state(&e, EE_EBRAKE_STATE_PRESSED);

  // set ebrake state -> set precharge
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_set_discharge(&e);

  // set precharge -> parking
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_PARKING);

  e.id = DRIVE_FSM_INPUT_EVENT_DRIVE;
  // parking -> set precharge
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_set_precharge(&e);

  // set precharge -> set ebrake
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_ebrake_state(&e, EE_EBRAKE_STATE_RELEASED);

  // set ebrake ->  neutral precharged
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_DRIVE, 0);

  // neutral precharged -> set mci output
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_mci_output(&e, EE_DRIVE_OUTPUT_DRIVE);

  // set mci output -> drive
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_DRIVE);

  e.id = DRIVE_FSM_INPUT_EVENT_REVERSE;

  // drive -> set mci output
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_mci_output(&e, EE_DRIVE_OUTPUT_REVERSE);

  // set mci output -> reverse
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_REVERSE);

  e.id = DRIVE_FSM_INPUT_EVENT_NEUTRAL;
  // reverse -> set mci state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_mci_output(&e, EE_DRIVE_OUTPUT_OFF);

  // set mci state -> neutral precharged
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_NEUTRAL);
}

void test_transition_to_fault_from_mci_output_and_recover_to_neutral(void) {
  // neutral discharged-> neutral precharged -> set mci output -> fault -> neutral -> parking ->
  // -> set precharge -> set ebrake -> fault -> parking
  TEST_ASSERT_OK(drive_fsm_init(&s_drive_fsm));

  Event e = { .id = DRIVE_FSM_INPUT_EVENT_NEUTRAL, .data = NUM_DRIVE_STATES };

  // neutral discharged -> set precharge
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_set_precharge(&e);

  // set precharge -> set ebrake
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_ebrake_state(&e, EE_EBRAKE_STATE_RELEASED);

  // set ebrake -> neutral precharged
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_NEUTRAL, 0);

  // neutral precharged -> set mci output
  e.id = DRIVE_FSM_INPUT_EVENT_DRIVE;
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_fail_mci_output(&e, EE_DRIVE_OUTPUT_DRIVE);

  // set mci output -> fault
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_recover_from_fault(&e, EE_EBRAKE_STATE_RELEASED);

  // fault -> neutral discharged
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // neutral discharged -> set ebrake
  e.id = DRIVE_FSM_INPUT_EVENT_PARKING;
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_ebrake_state(&e, EE_EBRAKE_STATE_PRESSED);

  // set ebrake -> set precharge
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_set_discharge(&e);

  // set precharge -> parking
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_output_event(DRIVE_FSM_OUTPUT_EVENT_PARKING);

  e.id = DRIVE_FSM_INPUT_EVENT_REVERSE;
  // parking -> set precharge
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_assert_set_precharge(&e);

  // set precharge -> set ebrake
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_fail_ebrake_state(&e, EE_EBRAKE_STATE_RELEASED);

  // set ebrake -> fault
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_recover_from_fault(&e, EE_EBRAKE_STATE_PRESSED);

  // fault -> parking
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_transition_to_ebrake_fault_from_parking_and_recover_to_neutral(void) {
  // neutral -> set ebrake -> parking -> set ebrake -> ebrake fault -> neutral
  TEST_ASSERT_OK(drive_fsm_init(&s_drive_fsm));

  Event e = { .id = DRIVE_FSM_INPUT_EVENT_PARKING, .data = NUM_DRIVE_STATES };

  // set precharge -> set ebrake
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_fail_ebrake_state(&e, EE_EBRAKE_STATE_PRESSED);

  // set ebrake -> fault
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  prv_recover_from_fault(&e, EE_EBRAKE_STATE_RELEASED);

  // fault -> neutral discharged
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
