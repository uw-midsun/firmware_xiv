#include "misc.h"
#include "pqueue.h"
#include "status.h"
#include "unity.h"

#define TEST_PQUEUE_SIZE 15

static PQueue gv_queue;
static PQueueNode gv_nodes[TEST_PQUEUE_SIZE];

void setup_test(void) {
  pqueue_init(&gv_queue, gv_nodes, TEST_PQUEUE_SIZE);
}

void teardown_test(void) {}

void test_pqueue_desc_order(void) {
  uint16_t prios[] = { 50, 10, 20, 2, 17, 5, 4000, 0, 3, 240 };

  for (size_t i = 0; i < SIZEOF_ARRAY(prios); i++) {
    pqueue_push(&gv_queue, &prios[i], prios[i]);
  }

  uint16_t last_prio = 0;
  for (size_t i = 0; i < SIZEOF_ARRAY(prios); i++) {
    uint16_t *prio = pqueue_pop(&gv_queue);
    TEST_ASSERT_MESSAGE(last_prio <= *prio, "Last priority was lower than new priority!");
    last_prio = *prio;
  }
}

void test_pqueue_out_of_space(void) {
  // We're using a 1-indexed minheap, so only expect success up to (# nodes - 1)
  for (size_t i = 0; i < TEST_PQUEUE_SIZE + 5; i++) {
    StatusCode result = pqueue_push(&gv_queue, (void *)i, i);
    if ((i + 1) < TEST_PQUEUE_SIZE) {
      TEST_ASSERT_EQUAL(STATUS_CODE_OK, result);
    } else {
      TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, result);
    }
  }

  // If we attempt to continue popping data after clearing the queue, expect
  // NULL.
  for (size_t i = 0; i < TEST_PQUEUE_SIZE + 5; i++) {
    TEST_ASSERT_EQUAL(((i + 1) < TEST_PQUEUE_SIZE) ? (void *)i : NULL, pqueue_pop(&gv_queue));
  }
}
