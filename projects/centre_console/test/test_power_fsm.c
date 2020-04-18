#include <string.h>
#include "centre_console_events.h"
#include "event_queue.h"
#include "log.h"
#include "power_fsm.h"
#include "unity.h"

static PowerFsmStorage s_power_fsm_storage;

void setup_test(void) {
  event_queue_init();
  memset(&s_power_fsm_storage, 0, sizeof(s_power_fsm_storage));
  power_fsm_init(&s_power_fsm_storage);
}

void teardown_test(void) {}

void prv_assert_current_state(PowerState current_state) {
  TEST_ASSERT_EQUAL(current_state, power_fsm_get_current_state(&s_power_fsm_storage));
}

void test_turn_on_event_begins_turn_on_main_battery_process(void) {
  prv_assert_current_state(POWER_STATE_OFF);
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, .data = 0 };

  power_fsm_process_event(&s_power_fsm_storage, &e);

  event_process(&e);
  TEST_ASSERT_EQUAL(POWER_MAIN_SEQUENCE_EVENT_BEGIN, e.id);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);
}

void test_fault_during_turn_on_transitions_to_fault_state(void) {
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, .data = 0 };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);
  event_process(&e);
  uint8_t fault_bit = 1 << 2;
  e.id = CENTRE_CONSOLE_POWER_EVENT_FAULT;
  e.data = fault_bit;

  power_fsm_process_event(&s_power_fsm_storage, &e);
  prv_assert_current_state(POWER_STATE_FAULT);

  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);
  TEST_ASSERT_EQUAL(fault_bit, e.data);
}

void test_fault_during_turn_off_transitions_to_fault_state(void) {
  // given: state_off -> state_turning_on -> state_on -> state_turning_off -> state_fault
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, .data = 0 };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);
  event_process(&e);

  e.id = POWER_MAIN_SEQUENCE_EVENT_COMPLETE;
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);
  prv_assert_current_state(POWER_STATE_MAIN);

  e.id = CENTRE_CONSOLE_POWER_EVENT_OFF;
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);

  e.id = CENTRE_CONSOLE_POWER_EVENT_OFF;
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);

  uint8_t fault_bit = 1 << 2;
  e.id = CENTRE_CONSOLE_POWER_EVENT_FAULT;
  e.data = fault_bit;

  // when
  power_fsm_process_event(&s_power_fsm_storage, &e);

  // then
  prv_assert_current_state(POWER_STATE_FAULT);
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);
  TEST_ASSERT_EQUAL(fault_bit, e.data);
}

void test_faults_accumulate(void) {
  // given
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_FAULT, .data = 1 << 1 };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);
  prv_assert_current_state(POWER_STATE_FAULT);

  uint8_t fault_bit = 1 << 2;
  e.id = CENTRE_CONSOLE_POWER_EVENT_FAULT;
  e.data = fault_bit;

  // when
  power_fsm_process_event(&s_power_fsm_storage, &e);

  // then
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);
  TEST_ASSERT_EQUAL(1 << 1 | 1 << 2, e.data);
}

void test_faults_clear_results_in_aux_transition(void) {
  uint8_t fault_bit = 1 << 1;
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_FAULT, .data = fault_bit };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  prv_assert_current_state(POWER_STATE_FAULT);
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);

  e.id = CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT;
  e.data = fault_bit;
  power_fsm_process_event(&s_power_fsm_storage, &e);

  event_process(&e);
  TEST_ASSERT_EQUAL(POWER_AUX_SEQUENCE_EVENT_BEGIN, e.id);
  power_fsm_process_event(&s_power_fsm_storage, &e);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);
}

void test_from_aux_to_main_begins_main_transition(void) {
  // given
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON_AUX, .data = 0 };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);
  TEST_ASSERT_EQUAL(POWER_AUX_SEQUENCE_EVENT_BEGIN, e.id);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);

  e.id = POWER_AUX_SEQUENCE_EVENT_COMPLETE;
  power_fsm_process_event(&s_power_fsm_storage, &e);
  prv_assert_current_state(POWER_STATE_AUX);

  e.id = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN;
  power_fsm_process_event(&s_power_fsm_storage, &e);

  event_process(&e);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);
  TEST_ASSERT_EQUAL(POWER_MAIN_SEQUENCE_EVENT_BEGIN, e.id);
}

void test_only_correct_fault_bit_clears(void) {
  // given
  uint8_t fault_bit = 1 << 1 | 1 << 2;
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_FAULT, .data = fault_bit };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);
  prv_assert_current_state(POWER_STATE_FAULT);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);

  e.id = CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT;
  e.data = 1 << 1;

  // when
  power_fsm_process_event(&s_power_fsm_storage, &e);

  // then
  prv_assert_current_state(POWER_STATE_FAULT);
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);
  TEST_ASSERT_EQUAL(1 << 2, e.data);
}
