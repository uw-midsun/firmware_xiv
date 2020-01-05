#include "unity.h"
#include "log.h"
#include "event_queue.h"
#include "centre_console_events.h"
#include "power_fsm.h"

static PowerFsmStorage s_power_fsm_storage;

void setup_test(void) {
  event_queue_init();
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

  uint8_t fault_bit = 1 << 2;
  e.id = CENTRE_CONSOLE_POWER_EVENT_FAULT;
  e.data = fault_bit;
  
  // when
  power_fsm_process_event(&s_power_fsm_storage, &e);

  // then
  event_process(&e);
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, e.id);
  TEST_ASSERT_EQUAL(fault_bit, e.data);
}


