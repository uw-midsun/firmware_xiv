#include "event_queue.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  event_queue_init();
}

void teardown_test(void) {}

// Test a single priority level.
void test_event_queue_raise(void) {
  // Fill the event queue
  Event events[EVENT_QUEUE_SIZE] = { { 0 } };
  for (int i = 0; i < EVENT_QUEUE_SIZE; i++) {
    events[i].id = i;
    events[i].data = i * 100;
    TEST_ASSERT_OK(event_raise(events[i].id, events[i].data));
  }

  // Attempt to insert an element when full
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, event_raise(NUM_EVENT_PRIORITIES, 0));

  Event e;
  uint16_t i = 0;
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_EQUAL(events[i].id, e.id);
    TEST_ASSERT_EQUAL(events[i].data, e.data);
    i++;
  }

  TEST_ASSERT_OK(event_raise(5, 5 * 100));
  TEST_ASSERT_OK(event_process(&e));

  TEST_ASSERT_EQUAL(5, e.id);
  TEST_ASSERT_EQUAL(5 * 100, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_event_queue_raise_priority(void) {
  // Test each priority.
  for (int i = NUM_EVENT_PRIORITIES - 1; i >= 0; i--) {
    TEST_ASSERT_OK(event_raise_priority((EventPriority)i, i * 10, i * 100));
  }

  Event e;
  uint16_t i = 0;
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_EQUAL(i * 10, e.id);
    TEST_ASSERT_EQUAL(i * 100, e.data);
    i++;
  }
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Spot check.
  TEST_ASSERT_OK(event_raise_priority(EVENT_PRIORITY_NORMAL, 1, 10));
  TEST_ASSERT_OK(event_raise_priority(EVENT_PRIORITY_HIGHEST, 1, 11));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(1, e.id);
  TEST_ASSERT_EQUAL(11, e.data);
  TEST_ASSERT_OK(event_raise_priority(EVENT_PRIORITY_HIGHEST, 1, 12));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(1, e.id);
  TEST_ASSERT_EQUAL(12, e.data);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(1, e.id);
  TEST_ASSERT_EQUAL(10, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
