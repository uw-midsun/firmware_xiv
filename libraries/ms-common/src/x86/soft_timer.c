// Timers are implemented as a doubly linked list, sorted in order of remaining
// time until expiry. Thus, the head of the list is always defined as the next
// timer to expire, so we back the list with a linux interval timer that's always
// set to the head's remaining time.
#include "soft_timer.h"

#include <signal.h>
#include <string.h>
#include <time.h>

#include "critical_section.h"
#include "interrupt_def.h"
#include "log.h"
#include "objpool.h"
#include "status.h"
#include "x86_interrupt.h"

#define SOFT_TIMER_GET_ID(timer) ((timer)-s_storage)

typedef struct SoftTimer {
  uint32_t remainder;
  SoftTimerCallback callback;
  void *context;
  struct SoftTimer *next;
  struct SoftTimer *prev;
} SoftTimer;

typedef struct SoftTimerList {
  SoftTimer *head;
  ObjectPool pool;
  timer_t timer_id;  // Single linux timer to back our soft timers
  bool created;      // bool for tracking timer creation in case of re-initialization
  struct sigevent event;
} SoftTimerList;

static SoftTimerList s_list = { 0 };

// Backing array for timer list objpool
static volatile SoftTimer s_storage[SOFT_TIMER_MAX_TIMERS] = { 0 };

// Set the linux timer to a new value. Checks for divide by zero.
static void prv_set_timer(uint32_t val_us) {
  // See timer_settime manual page for details
  struct itimerspec spec = {
    { 0, 0 },                                       //
    { val_us / 1000000, val_us % 1000000 * 1000 },  //
  };
  timer_settime(s_list.timer_id, 0, &spec, NULL);
}

// Helper function for removing a cancelled or triggered timer from the list.
// Expects to be wrapped in a critical section.
static void prv_remove_timer(SoftTimer *node) {
  if (node == s_list.head) {
    // Get the actual elapsed time from the head timer in case it didn't complete (was cancelled)
    uint32_t elapsed =
        s_list.head->remainder - soft_timer_remaining_time(SOFT_TIMER_GET_ID(s_list.head));
    // Keep the rest of the timers up to date with current the head
    SoftTimer *cur = s_list.head->next;
    while (cur != NULL) {
      cur->remainder -= elapsed;
      cur = cur->next;
    }

    s_list.head = node->next;
  }
  if (node->prev != NULL) {
    node->prev->next = node->next;
  }
  if (node->next != NULL) {
    node->next->prev = node->prev;
  }

  // Update the linux timer to the new head, or cancel it (set to 0) if the remainder is 0.
  // The remainder can be zero if two timers were initialized with the same value.
  prv_set_timer(s_list.head ? s_list.head->remainder : 0);

  objpool_free_node(&s_list.pool, node);
}

// Interrupt handler for the timer signal
static void prv_soft_timer_handler(uint8_t interrupt_id) {
  SoftTimer *cur = s_list.head;
  if (cur == NULL) {
    return;
  }
  bool crit = critical_section_start();
  // Always handle the triggered timer, then handle any following timers that were started
  // at the same time with the same value as the first timer.
  do {
    SoftTimer temp = *s_list.head;
    SoftTimerId temp_id = SOFT_TIMER_GET_ID(s_list.head);
    // Remove the timer before calling the callback in case the callback starts a new timer,
    // prv_remove_timer() updates all time values in the list.
    prv_remove_timer(s_list.head);
    temp.callback(temp_id, temp.context);
    cur = s_list.head;
  } while (cur != NULL && cur->remainder == 0);
  critical_section_end(crit);
}

void soft_timer_init(void) {
  s_list.head = NULL;
  objpool_init(&s_list.pool, s_storage, NULL, NULL);
  if (s_list.created) {
    timer_delete(s_list.timer_id);
    s_list.created = false;
  }

  // Register a handler and interrupt.
  uint8_t handler_id;
  x86_interrupt_register_handler(prv_soft_timer_handler, &handler_id);
  InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };
  uint8_t interrupt_id;
  x86_interrupt_register_interrupt(handler_id, &it_settings, &interrupt_id);

  // Create the event to trigger on.
  s_list.event.sigev_value.sival_int = interrupt_id;
  s_list.event.sigev_notify = SIGEV_SIGNAL;
  s_list.event.sigev_signo = SIGRTMIN + INTERRUPT_PRIORITY_NORMAL;

  timer_create(CLOCK_MONOTONIC, &s_list.event, &s_list.timer_id);
  s_list.created = true;
}

StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void *context,
                            SoftTimerId *timer_id) {
  if (duration_us < SOFT_TIMER_MIN_TIME_US) {
    return STATUS_CODE_INVALID_ARGS;
  }
  SoftTimer *node = objpool_get_node(&s_list.pool);
  if (node == NULL) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
  memset(node, 0, sizeof(*node));

  node->remainder = duration_us;
  node->callback = callback;
  node->context = context;

  if (timer_id != NULL) {
    *timer_id = SOFT_TIMER_GET_ID(node);
  }

  bool crit = critical_section_start();
  SoftTimer *next = s_list.head;
  SoftTimer *prev = NULL;
  while (next != NULL && next->remainder <= node->remainder) {
    prev = next;
    next = next->next;
  }
  if (prev == NULL) {
    s_list.head = node;
    prv_set_timer(node->remainder);
  } else {
    prev->next = node;
    node->prev = prev;
  }
  if (next != NULL) {
    next->prev = node;
    node->next = next;
  }
  critical_section_end(crit);
  return STATUS_CODE_OK;
}

bool soft_timer_cancel(SoftTimerId timer_id) {
  if (timer_id >= SOFT_TIMER_MAX_TIMERS || !soft_timer_inuse()) {
    return false;
  }

  bool crit = critical_section_start();
  prv_remove_timer(&s_storage[timer_id]);
  critical_section_end(crit);
  return true;
}

bool soft_timer_inuse(void) {
  return s_list.head != NULL;
}

uint32_t soft_timer_remaining_time(SoftTimerId timer_id) {
  if (timer_id >= SOFT_TIMER_MAX_TIMERS || !soft_timer_inuse()) {
    return 0;
  }

  // See timer_gettime manual page for details
  struct itimerspec spec = { { 0, 0 }, { 0, 0 } };
  timer_gettime(s_list.timer_id, &spec);
  uint32_t head_remaining_us = spec.it_value.tv_sec * 1000000 + spec.it_value.tv_nsec / 1000;

  return s_storage[timer_id].remainder - s_list.head->remainder + head_remaining_us;
}
