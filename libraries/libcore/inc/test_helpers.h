#pragma once
// Test helper functions. These should only ever be called within a file in the
// test folder.

#include "status.h"
#include "unity.h"

// General use:
#define TEST_ASSERT_OK(code) TEST_ASSERT_EQUAL(STATUS_CODE_OK, (code))
#define TEST_ASSERT_NOT_OK(code) TEST_ASSERT_NOT_EQUAL(STATUS_CODE_OK, (code))

// Mocking
#define TEST_MOCK(func) __attribute__((used)) __wrap_##func

// Add '#define DETERMINISTIC_SOFT_TIMERS' to the top of your test file
// as well as the appropriate mocks in the makefile to use.
// WARNING: Has issues if your code uses critical sections.
#ifdef DETERMINISTIC_SOFT_TIMERS

#include <string.h>

#include "delay.h"
#include "soft_timer.h"

typedef struct DeterministicTimer {
  SoftTimerCallback callback;
  void *context;
  bool inuse;
  uint32_t time_remaining;
} DeterministicTimer;

// indexed by SoftTimerId
static DeterministicTimer s_timers[SOFT_TIMER_MAX_TIMERS];
static uint8_t s_active_timers = 0;

void prv_trigger_timer(DeterministicTimer *timer, SoftTimerId timer_id) {
  timer->callback(timer_id, timer->context);
  memset(timer, 0, sizeof(*timer));
  s_active_timers--;
}

void prv_timers_tick(void) {
  for (SoftTimerId i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (s_timers[i].inuse) {
      s_timers[i].time_remaining--;
      if (s_timers[i].time_remaining == 0) {
        prv_trigger_timer(&s_timers[i], i);
      }
    }
  }
}

StatusCode TEST_MOCK(soft_timer_start)(uint32_t duration_us, SoftTimerCallback callback,
                                       void *context, SoftTimerId *timer_id) {
  if (duration_us < SOFT_TIMER_MIN_TIME_US) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Soft timer too short!");
  }

  for (SoftTimerId i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (s_timers[i].inuse == false) {
      s_timers[i].callback = callback;
      s_timers[i].context = context;
      s_timers[i].time_remaining = duration_us;
      s_timers[i].inuse = true;

      if (timer_id != NULL) {
        *timer_id = i;
      }
      s_active_timers++;
      return STATUS_CODE_OK;
    }
  }

  return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
}

bool TEST_MOCK(soft_timer_cancel)(SoftTimerId timer_id) {
  if (timer_id < SOFT_TIMER_MAX_TIMERS) {
    if (s_timers[timer_id].inuse == true) {
      memset(&s_timers[timer_id], 0, sizeof(s_timers[timer_id]));
      s_active_timers--;
      return true;
    }
  }
  return false;
}

bool TEST_MOCK(soft_timer_inuse)(void) {
  if (s_active_timers > 0) {
    return true;
  }
  return false;
}

void TEST_MOCK(soft_timer_init)(void) {
  s_active_timers = 0;
  memset(&s_timers[0], 0, sizeof(s_timers));
}

void TEST_MOCK(delay_us)(uint32_t t) {
  for (uint32_t i = 0; i < t; i++) {
    prv_timers_tick();
  }
}

#endif
