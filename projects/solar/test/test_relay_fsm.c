#include "relay_fsm.h"

#include <stdbool.h>

#include "can.h"
#include "can_transmit.h"
#include "delay.h"
#include "drv120_relay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

#define IRRELEVANT_EVENT NUM_SOLAR_RELAY_EVENTS

#define PROCESS_RELAY_FSM_EVENT(storage, e)    \
  ({                                           \
    MS_TEST_HELPER_AWAIT_EVENT(e);             \
    relay_fsm_process_event(&(storage), &(e)); \
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();   \
  })

static const GpioAddress s_test_relay_pin = { .port = GPIO_PORT_A, .pin = 6 };

static CanStorage s_can_storage;
static RelayFsmStorage s_storage;

void setup_test(void) {
  event_queue_init();
  drv120_relay_init(&s_test_relay_pin);
  TEST_ASSERT_OK(relay_fsm_init(&s_storage));
}
void teardown_test(void) {}

// Test that the relay starts closed and is changed in response to command functions.
void test_relay_transitions(void) {
  Event e = { 0 };
  bool closed;

  // starts closed
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed -> open
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed -> open
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open still (should be no-op)
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed still (should be no-op)
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
}

// Test that the FSM doesn't respond to irrelevant events.
void test_irrelevant_events(void) {
  Event e = { 0 };
  bool closed;

  // reset to open
  TEST_ASSERT_OK(relay_fsm_open());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  e.id = IRRELEVANT_EVENT;
  relay_fsm_process_event(&s_storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);  // not changed
}

// Test that |relay_fsm_process_event| returns whether there was a state change.
void test_relay_fsm_process_event_return_value(void) {
  // force to closed at start - valid in case we don't initialize to closed in the future
  Event e = { 0 };
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);

  e.id = IRRELEVANT_EVENT;
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, &e));

  TEST_ASSERT_OK(relay_fsm_open());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_TRUE(relay_fsm_process_event(&s_storage, &e));  // now open

  TEST_ASSERT_OK(relay_fsm_open());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, &e));  // still open - no change

  TEST_ASSERT_OK(relay_fsm_close());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_TRUE(relay_fsm_process_event(&s_storage, &e));  // now closed

  TEST_ASSERT_OK(relay_fsm_close());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, &e));  // still closed - no change

  e.id = IRRELEVANT_EVENT;
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, &e));
}

// Test that the module gracefully handles null and invalid inputs.
void test_invalid_input(void) {
  TEST_ASSERT_NOT_OK(relay_fsm_init(NULL));

  // force to closed so we can get a state changing event
  Event e = { 0 };
  TEST_ASSERT_OK(relay_fsm_close());
  PROCESS_RELAY_FSM_EVENT(s_storage, e);

  TEST_ASSERT_OK(relay_fsm_open());
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_FALSE(relay_fsm_process_event(NULL, &e));
  TEST_ASSERT_FALSE(relay_fsm_process_event(&s_storage, NULL));
  TEST_ASSERT_FALSE(relay_fsm_process_event(NULL, NULL));
}
