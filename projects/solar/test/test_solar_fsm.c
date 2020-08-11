#include "solar_fsm.h"

#include <stdbool.h>

#include "drv120_relay.h"
#include "event_queue.h"
#include "gpio.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

#define UNRECOGNIZED_EVENT NUM_SOLAR_EXTERNAL_COMMAND_EVENTS
#define TEST_FAULT_EVENT(n) (NUM_SOLAR_EXTERNAL_COMMAND_EVENTS + 1 + (n))

static const GpioAddress s_test_relay_pin = { .port = GPIO_PORT_A, .pin = 6 };

static const Event s_close_command_event = { .id = SOLAR_COMMAND_EVENT_CLOSE_RELAY };
static const Event s_open_command_event = { .id = SOLAR_COMMAND_EVENT_OPEN_RELAY };

void setup_test(void) {
  event_queue_init();
  gpio_init();
  drv120_relay_init(&s_test_relay_pin);
}
void teardown_test(void) {}

// Test that the relay starts out closed, and a single fault event causes it to open.
void test_relay_opening_single_fault_event(void) {
  bool closed;
  SolarFsmSettings settings = {
    .relay_open_events = { TEST_FAULT_EVENT(0) },
    .num_relay_open_events = 1,
  };
  SolarFsmStorage storage;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  // should start off with relay closed
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // raise an unrecognized event - nothing should happen
  Event e = { .id = UNRECOGNIZED_EVENT };
  solar_fsm_process_event(&storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // raising the fault event opens the relay
  e.id = TEST_FAULT_EVENT(0);
  solar_fsm_process_event(&storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // raising unrecognized/fault events don't cause it to reclose
  e.id = UNRECOGNIZED_EVENT;
  solar_fsm_process_event(&storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);
  e.id = TEST_FAULT_EVENT(0);
  solar_fsm_process_event(&storage, &e);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);
}

// Test that the FSM opens/closes the relay correctly in response to command events.
void test_responding_command_events(void) {
  bool closed;
  SolarFsmSettings settings = {
    .relay_open_events = { TEST_FAULT_EVENT(0) },
    .num_relay_open_events = 1,
  };
  SolarFsmStorage storage;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  // open
  solar_fsm_process_event(&storage, &s_open_command_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  solar_fsm_process_event(&storage, &s_close_command_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed -> open
  solar_fsm_process_event(&storage, &s_open_command_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open still (should be no-op)
  solar_fsm_process_event(&storage, &s_open_command_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_FALSE(closed);

  // open -> closed
  solar_fsm_process_event(&storage, &s_close_command_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // closed still (should be no-op)
  solar_fsm_process_event(&storage, &s_close_command_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
}

// Test that the maximum number of fault events can all open the relay. We use command events to
// reclose the relay after each opening.
void test_relay_opening_multiple_fault_events(void) {
  bool closed;
  SolarFsmSettings settings = {
    .num_relay_open_events = MAX_RELAY_OPEN_EVENTS,
  };
  for (uint8_t i = 0; i < MAX_RELAY_OPEN_EVENTS; i++) {
    settings.relay_open_events[i] = TEST_FAULT_EVENT(i);
  }
  SolarFsmStorage storage;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  // should start off with relay closed
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);

  // try each fault event
  Event fault_event = { 0 };
  for (uint8_t i = 0; i < MAX_RELAY_OPEN_EVENTS; i++) {
    fault_event.id = TEST_FAULT_EVENT(i);
    solar_fsm_process_event(&storage, &fault_event);
    drv120_relay_get_is_closed(&closed);
    TEST_ASSERT_FALSE(closed);

    // reset using the command event
    solar_fsm_process_event(&storage, &s_close_command_event);
    drv120_relay_get_is_closed(&closed);
    TEST_ASSERT_TRUE(closed);
  }

  // events that aren't mentioned don't open the relay
  fault_event.id = TEST_FAULT_EVENT(MAX_RELAY_OPEN_EVENTS);
  solar_fsm_process_event(&storage, &fault_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
  fault_event.id = UNRECOGNIZED_EVENT;
  solar_fsm_process_event(&storage, &fault_event);
  drv120_relay_get_is_closed(&closed);
  TEST_ASSERT_TRUE(closed);
}

// Test that |solar_fsm_process_event| returns whether there was a state change.
void test_solar_fsm_process_event_return_value(void) {
  SolarFsmSettings settings = {
    .relay_open_events = { TEST_FAULT_EVENT(0), TEST_FAULT_EVENT(1) },
    .num_relay_open_events = 2,
  };
  SolarFsmStorage storage;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  // force to closed at start - valid in case we don't initialize to closed in the future
  solar_fsm_process_event(&storage, &s_close_command_event);

  Event e = { .id = UNRECOGNIZED_EVENT };
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &e));

  e.id = TEST_FAULT_EVENT(0);
  TEST_ASSERT_TRUE(solar_fsm_process_event(&storage, &e));  // now open
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &e));

  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &s_open_command_event));
  TEST_ASSERT_TRUE(solar_fsm_process_event(&storage, &s_close_command_event));  // now closed
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &s_close_command_event));

  e.id = TEST_FAULT_EVENT(1);
  TEST_ASSERT_TRUE(solar_fsm_process_event(&storage, &e));  // now open

  e.id = UNRECOGNIZED_EVENT;
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &e));

  TEST_ASSERT_TRUE(solar_fsm_process_event(&storage, &s_close_command_event));  // now closed
  TEST_ASSERT_TRUE(solar_fsm_process_event(&storage, &s_open_command_event));   // now open
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, &s_open_command_event));
}

// Test that the module gracefully handles null and invalid inputs.
void test_invalid_input(void) {
  SolarFsmSettings settings = {
    .relay_open_events = { TEST_FAULT_EVENT(0) },
    .num_relay_open_events = 1,
  };
  SolarFsmStorage storage;
  TEST_ASSERT_NOT_OK(solar_fsm_init(NULL, &settings));
  TEST_ASSERT_NOT_OK(solar_fsm_init(&storage, NULL));
  TEST_ASSERT_NOT_OK(solar_fsm_init(NULL, NULL));

  settings.num_relay_open_events = MAX_RELAY_OPEN_EVENTS + 1;
  TEST_ASSERT_NOT_OK(solar_fsm_init(&storage, &settings));

  // valid initialization for testing |solar_fsm_process_event|
  settings.num_relay_open_events = 1;
  TEST_ASSERT_OK(solar_fsm_init(&storage, &settings));

  Event e = { .id = TEST_FAULT_EVENT(0) };
  TEST_ASSERT_FALSE(solar_fsm_process_event(NULL, &e));
  TEST_ASSERT_FALSE(solar_fsm_process_event(&storage, NULL));
  TEST_ASSERT_FALSE(solar_fsm_process_event(NULL, NULL));
}
