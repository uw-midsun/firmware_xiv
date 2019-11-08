#include "fsm.h"
#include "log.h"
#include "unity.h"

typedef enum {
  TEST_FSM_EVENT_A = 0,
  TEST_FSM_EVENT_B,
  TEST_FSM_EVENT_C,
} TEST_FSM_EVENT;

static Fsm s_fsm;
static uint16_t s_num_output;

static bool prv_guard(const Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("Hit transition guard - returning %d\n", (bool)e->data);
  return (bool)e->data;
}

FSM_DECLARE_STATE(test_a);
FSM_DECLARE_STATE(test_b);
FSM_DECLARE_STATE(test_c);

FSM_STATE_TRANSITION(test_a) {
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_A, test_a);
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_B, test_b);
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_C, test_c);
}

FSM_STATE_TRANSITION(test_b) {
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_C, test_c);
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_A, test_a);
}

FSM_STATE_TRANSITION(test_c) {
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_C, test_c);
  FSM_ADD_GUARDED_TRANSITION(TEST_FSM_EVENT_B, prv_guard, test_a);
}

static void prv_output(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("[%s:%s] State reached from %s (Event %d, data %d)\n", fsm->name,
            fsm->current_state->name, fsm->last_state->name, e->id, e->data);
  TEST_ASSERT_EQUAL(fsm, context);
  s_num_output++;
}

void setup_test(void) {
  fsm_state_init(test_c, prv_output);
  fsm_init(&s_fsm, "test_fsm", &test_a, &s_fsm);
  s_num_output = 0;
}

void teardown_test(void) {}

void test_fsm_transition(void) {
  Event e = {
    .id = TEST_FSM_EVENT_A,  //
    .data = 10,              //
  };

  // Expect A -> A -> B -> fail (B) -> C (output) -> C (output) -> fail (C)
  bool transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);

  e.id = TEST_FSM_EVENT_B;
  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);

  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_FALSE(transitioned);

  e.id = TEST_FSM_EVENT_C;
  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);
  TEST_ASSERT_EQUAL(1, s_num_output);

  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);
  TEST_ASSERT_EQUAL(2, s_num_output);

  e.id = TEST_FSM_EVENT_A;
  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_FALSE(transitioned);
}

void test_fsm_guard(void) {
  Event e = {
    .id = TEST_FSM_EVENT_C,  //
    .data = false,           //
  };

  // Expect A -> C -> guard fail (C) -> B
  bool transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);

  e.id = TEST_FSM_EVENT_B;
  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_FALSE(transitioned);

  e.data = true;
  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);
}
