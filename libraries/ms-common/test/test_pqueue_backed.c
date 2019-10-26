#include "pqueue_backed.h"
#include "unity.h"

// Note that event_queue is a wrapper for pqueue_backed

#define TEST_PQUEUE_BACKED_SIZE 10

typedef struct TestObject {
  uint16_t data;
} TestObject;

static PQueueBacked s_queue;
static PQueueNode s_nodes[TEST_PQUEUE_BACKED_SIZE + 1];
static TestObject s_elems[TEST_PQUEUE_BACKED_SIZE];

void setup_test(void) {
  pqueue_backed_init(&s_queue, s_nodes, s_elems);
}

void teardown_test(void) {}

void test_pqueue_backed_run(void) {
  uint16_t prios[] = { 50, 10, 20, 2, 17, 5, 4000, 0, 3, 240 };

  for (size_t i = 0; i < SIZEOF_ARRAY(prios); i++) {
    StatusCode result = pqueue_backed_push(&s_queue, &(TestObject){ .data = prios[i] }, prios[i]);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, result);
  }

  uint16_t last_prio = 0;
  for (size_t i = 0; i < SIZEOF_ARRAY(prios); i++) {
    TestObject obj;
    StatusCode result = pqueue_backed_pop(&s_queue, &obj);
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, result);
    TEST_ASSERT_MESSAGE(last_prio <= obj.data, "Last data was lower than new data!");
    last_prio = obj.data;
  }
}
