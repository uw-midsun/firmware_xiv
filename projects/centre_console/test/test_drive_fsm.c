#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"
#include "drive_fsm.h"
#include "relay_tx.h"
#include "ebrake_tx.h"
#include "centre_console_fault_reason.h"

static DriveFsmStorage s_drive_fsm = { 0 };
static bool s_fault = false;

StatusCode TEST_MOCK(relay_tx_relay_state_and_raise_event)(RelayTxStorage *storage, RelayTxRequest *request) {
  if (s_fault) {
    event_raise(DRIVE_FSM_INPUT_EVENT_FAULT, request->fault_event_data);
  } else {
    LOG_DEBUG("setting relay state: %d\n", request->state);
    event_raise(request->completion_event_id, request->completion_event_data);
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(ebrake_tx_state_and_raise_event)(EbrakeTxStorage *storage, EbrakeTxRequest *request) {
  if (s_fault) {
    event_raise(DRIVE_FSM_INPUT_EVENT_FAULT, request->fault_event_data);
  } else {
    LOG_DEBUG("setting ebrake state: %d\n", request->state);
    event_raise(request->completion_event_id, request->completion_event_data);
  }
  return STATUS_CODE_OK;
}

void setup_test(void) {
  static bool s_fault = false;
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_transition_to_drive_then_parking_then_reverse(void) {
  // neutral -> drive -> parking -> reverse
  TEST_ASSERT_OK(drive_fsm_init(&s_drive_fsm));

  Event e = { .id = DRIVE_FSM_INPUT_EVENT_DRIVE, .data = NUM_DRIVE_STATES };
  // neutral -> set relay state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_DRIVE, DRIVE_STATE_DRIVE);

  // set relay states -> drive
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_DRIVE, 0);

  e.id = DRIVE_FSM_INPUT_EVENT_PARKING;

  // drive -> set relay state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_PARKING, DRIVE_STATE_PARKING);

  // set relay state -> set ebrake state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED, DRIVE_STATE_PARKING);

  // set ebrake state -> parking
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_PARKING, 0);

  // parking -> set ebrake state
  e.id = DRIVE_FSM_INPUT_EVENT_REVERSE;
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED_DESTINATION_DRIVE_REVERSE, DRIVE_STATE_REVERSE);

  // set ebrake state -> set relay state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE, DRIVE_STATE_REVERSE);

  // set relay state -> reverse
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_REVERSE, 0);
}

void test_transition_to_parking_drive_reverse_neutral_parking(void) {
  // neutral -> parking -> drive -> reverse -> neutral -> parking
  TEST_ASSERT_OK(drive_fsm_init(&s_drive_fsm));

  Event e = { .id = DRIVE_FSM_INPUT_EVENT_PARKING, .data = NUM_DRIVE_STATES };
  // neutral -> set ebrake state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED, DRIVE_STATE_PARKING);

  // set ebrake state -> parking
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_PARKING, 0);

  e.id = DRIVE_FSM_INPUT_EVENT_DRIVE;

  // parking -> set ebrake state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED_DESTINATION_DRIVE_REVERSE, DRIVE_STATE_DRIVE);

  // set ebrake state -> set relay state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_DRIVE, DRIVE_STATE_DRIVE);

  // set relay state -> drive 
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_DRIVE, 0);

  e.id = DRIVE_FSM_INPUT_EVENT_REVERSE;

  // drive -> reverse
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_REVERSE, 0);

  e.id = DRIVE_FSM_INPUT_EVENT_NEUTRAL;

  // reverse -> set relay state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_NEUTRAL, DRIVE_STATE_NEUTRAL);

  // set relay state -> neutral
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_NEUTRAL, 0);

  e.id = DRIVE_FSM_INPUT_EVENT_PARKING;
  // neutral -> set ebrake state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED, DRIVE_STATE_PARKING);

  // set ebrake state -> parking
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_PARKING, 0);
}

void test_transition_to_parking_drive_fault_clear_fault(void) {
  // neutral -> set relay state -> fault -> neutral -> set ebrake state -> fault -> neutral
  TEST_ASSERT_OK(drive_fsm_init(&s_drive_fsm));

  s_fault = true;

  Event e = { .id = DRIVE_FSM_INPUT_EVENT_DRIVE, .data = NUM_DRIVE_STATES };
  // neutral -> set relay state
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));

  DriveFsmFault fault = {
    .fault_reason = DRIVE_FSM_FAULT_REASON_MCI_RELAY_STATE,
    .fault_state = EE_RELAY_STATE_CLOSE
  };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_FAULT, fault.raw);

  // set relay state -> fault
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));

  // fault should attempt transitioning to neutral
  fault.fault_state = EE_RELAY_STATE_OPEN;
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_FAULT, fault.raw);

  s_fault = false;

  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_NEUTRAL, DRIVE_STATE_NEUTRAL);

  // fault -> neutral
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_OUTPUT_EVENT_NEUTRAL, 0);

  s_fault = true;

  // neutral -> set ebrake state
  e.id = DRIVE_FSM_INPUT_EVENT_PARKING;
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));

  fault.fault_reason = DRIVE_FSM_FAULT_REASON_EBRAKE_STATE;
  fault.fault_state = EE_EBRAKE_STATE_PRESSED;
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_FAULT, fault.raw);

  // set ebrake state -> fault
  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));

  fault.fault_reason = DRIVE_FSM_FAULT_REASON_MCI_RELAY_STATE;
  fault.fault_state = EE_RELAY_STATE_OPEN;
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_FAULT, fault.raw);

  s_fault = false;

  TEST_ASSERT_TRUE(drive_fsm_process_event(&s_drive_fsm, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_NEUTRAL, DRIVE_STATE_NEUTRAL);
}
