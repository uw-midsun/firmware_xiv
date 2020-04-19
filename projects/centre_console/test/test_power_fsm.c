#include <string.h>
#include "centre_console_events.h"
#include "event_queue.h"
#include "log.h"
#include "ms_test_helpers.h"
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
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));

  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, POWER_MAIN_SEQUENCE_EVENT_BEGIN);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);

  e.id = POWER_MAIN_SEQUENCE_EVENT_COMPLETE;
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));

  prv_assert_current_state(POWER_STATE_MAIN);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_fault_during_turn_on_transitions_to_fault_state_then_back_to_off(void) {
  // off -> transitioning -> fault -> off
  prv_assert_current_state(POWER_STATE_OFF);

  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, .data = 0 };
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));
  prv_assert_current_state(POWER_STATE_TRANSITIONING);

  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, POWER_MAIN_SEQUENCE_EVENT_BEGIN);

  uint16_t fault_data = 0x1234;
  e.id = CENTRE_CONSOLE_POWER_EVENT_FAULT;
  e.data = fault_data;

  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));
  prv_assert_current_state(POWER_STATE_FAULT);

  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, POWER_STATE_OFF);
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));

  prv_assert_current_state(POWER_STATE_OFF);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_fault_during_turn_off_transitions_back_to_main_state(void) {
  // given: off -> transitioning -> main -> transitioning -> fault -> main
  prv_assert_current_state(POWER_STATE_OFF);

  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, .data = 0 };
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));

  prv_assert_current_state(POWER_STATE_TRANSITIONING);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, POWER_MAIN_SEQUENCE_EVENT_BEGIN);

  e.id = POWER_MAIN_SEQUENCE_EVENT_COMPLETE;
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));

  prv_assert_current_state(POWER_STATE_MAIN);

  e.id = CENTRE_CONSOLE_POWER_EVENT_OFF;
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));

  prv_assert_current_state(POWER_STATE_TRANSITIONING);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, POWER_OFF_SEQUENCE_EVENT_BEGIN);

  uint8_t fault_data = 123;
  e.id = CENTRE_CONSOLE_POWER_EVENT_FAULT;
  e.data = fault_data;

  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));
  prv_assert_current_state(POWER_STATE_FAULT);

  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, POWER_STATE_MAIN);
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));

  prv_assert_current_state(POWER_STATE_MAIN);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_from_aux_to_main_begins_main_transition(void) {
  // off -> transitioning -> aux -> transitioning -> main
  prv_assert_current_state(POWER_STATE_OFF);

  Event e = { .id = CENTRE_CONSOLE_POWER_EVENT_ON_AUX };
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));

  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, POWER_AUX_SEQUENCE_EVENT_BEGIN);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  e.id = POWER_AUX_SEQUENCE_EVENT_COMPLETE;
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));
  prv_assert_current_state(POWER_STATE_AUX);

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  e.id = CENTRE_CONSOLE_POWER_EVENT_ON_MAIN;
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT_ID(e, POWER_MAIN_SEQUENCE_EVENT_BEGIN);
  prv_assert_current_state(POWER_STATE_TRANSITIONING);

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  e.id = POWER_MAIN_SEQUENCE_EVENT_COMPLETE;
  TEST_ASSERT_TRUE(power_fsm_process_event(&s_power_fsm_storage, &e));
  prv_assert_current_state(POWER_STATE_MAIN);

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
