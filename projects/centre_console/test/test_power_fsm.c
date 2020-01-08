#include "unity.h"
#include "log.h"
#include "event_queue.h"
#include "centre_console_events.h"
#include "power_fsm.h"
#include <string.h>

static PowerFsmStorage s_power_fsm_storage;

void setup_test(void) {
  event_queue_init();
  memset(&s_power_fsm_storage, 0, sizeof(s_power_fsm_storage));
  power_fsm_init(&s_power_fsm_storage);
}

void teardown_test(void) {}

void test_turn_on_event_begins_turn_on_process(void) {
  // given
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON, .data = 0 };
  
  // when
  power_fsm_process_event(&s_power_fsm_storage, &e);

  // then
  event_process(&e);
  TEST_ASSERT_EQUAL(POWER_ON_SEQUENCE_EVENT_BEGIN, e.id);
}

void test_fault_during_turn_on_transitions_to_fault_state(void) {
  // given
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON, .data = 0 };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);

  uint8_t fault_bit = 1 << 2;
  e.id = CENTRE_CONSOLE_POWER_EVENT_FAULT;
  e.data = fault_bit;
  
  // when
  power_fsm_process_event(&s_power_fsm_storage, &e);

  // then
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);
  TEST_ASSERT_EQUAL(fault_bit, e.data);
}

void test_fault_during_turn_off_transitions_to_fault_state(void) {
  // given: state_off -> state_turning_on -> state_on -> state_turning_off -> state_fault
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON, .data = 0 };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);

  e.id = POWER_ON_SEQUENCE_EVENT_COMPLETE;
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);

  e.id = CENTRE_CONSOLE_POWER_EVENT_OFF;
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);

  e.id = CENTRE_CONSOLE_POWER_EVENT_OFF;
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);

  uint8_t fault_bit = 1 << 2;
  e.id = CENTRE_CONSOLE_POWER_EVENT_FAULT;
  e.data = fault_bit;
  
  // when
  power_fsm_process_event(&s_power_fsm_storage, &e);

  // then
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);
  TEST_ASSERT_EQUAL(fault_bit, e.data);
}

void test_faults_accumulate(void) {
  // given
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_FAULT, .data = 1<<1 };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);

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

void test_faults_clear(void) {
  // given
  s_power_fsm_storage.fault_bitset = 0;
  uint8_t fault_bit = 1 << 1;
  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_FAULT, .data = fault_bit };
  power_fsm_process_event(&s_power_fsm_storage, &e);
  event_process(&e);

  e.id = CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT;
  e.data = fault_bit;
  
  // when
  power_fsm_process_event(&s_power_fsm_storage, &e);

  // then
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_FAULTS_CLEARED, e.id);
}




