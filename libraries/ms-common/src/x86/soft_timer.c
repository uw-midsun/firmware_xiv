#include "soft_timer.h"

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "critical_section.h"
#include "interrupt_def.h"
#include "status.h"
#include "x86_interrupt.h"

typedef struct POSIXTimer {
  timer_t timer_id;
  SoftTimerCallback callback;
  void *context;
  volatile bool inuse;
  bool created;
} POSIXTimer;

static struct sigevent s_event;
static POSIXTimer s_posix_timers[SOFT_TIMER_MAX_TIMERS];
static volatile uint8_t s_active_timers = 0;

static void prv_soft_timer_interrupt(void) {
  const bool critical = critical_section_start();
  struct itimerspec spec = { { 0, 0 }, { 0, 0 } };
  for (uint16_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (!s_active_timers) {
      // No active timers left.
      break;
    }
    if (s_posix_timers[i].inuse) {
      timer_gettime(s_posix_timers[i].timer_id, &spec);
      if (spec.it_value.tv_sec == 0 && spec.it_value.tv_nsec == 0) {
        // Mark as no longer inuse in case a timer is cancelled within its
        // callback resulting in underflow of |s_active_timers|.
        s_posix_timers[i].inuse = false;
        s_posix_timers[i].callback(i, s_posix_timers[i].context);
        s_active_timers--;
      }
    }
  }
  critical_section_end(critical);
}

static void prv_soft_timer_handler(uint8_t interrupt_id) {
  // Run the interrupt since there is only one for this handler.
  prv_soft_timer_interrupt();
}

void soft_timer_init(void) {
  // Set all timers to make none appear active.

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
  s_event.sigev_value.sival_int = interrupt_id;
  s_event.sigev_notify = SIGEV_SIGNAL;
  s_event.sigev_signo = SIGRTMIN + INTERRUPT_PRIORITY_NORMAL;

  // Clear all the statics and reset all the clocks.
  s_active_timers = 0;
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    s_posix_timers[i].inuse = false;
    if (s_posix_timers[i].created) {
      timer_delete(s_posix_timers[i].timer_id);
    }
    timer_create(CLOCK_MONOTONIC, &s_event, &s_posix_timers[i].timer_id);
    s_posix_timers[i].created = true;
  }
}

StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void *context,
                            SoftTimerId *timer_id) {
  if (duration_us < SOFT_TIMER_MIN_TIME_US) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Soft timer too short!");
  }
  // Start a critical section to prevent this section from being broken.
  const bool critical = critical_section_start();
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (!s_posix_timers[i].inuse) {
      // Look for an empty timer.

      // Set the timer with it_val.
      struct itimerspec spec = { { 0, 0 },
                                 { duration_us / 1000000, duration_us % 1000000 * 1000 } };
      timer_settime(s_posix_timers[i].timer_id, 0, &spec, NULL);
      s_posix_timers[i].inuse = true;
      s_posix_timers[i].context = context;
      s_posix_timers[i].callback = callback;
      if (timer_id != NULL) {
        *timer_id = i;
      }
      s_active_timers++;
      critical_section_end(critical);
      return STATUS_CODE_OK;
    }
  }

  // Out of timers.
  critical_section_end(critical);
  return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
}

bool soft_timer_inuse(void) {
  if (s_active_timers > 0) {
    return true;
  }
  return false;
}

bool soft_timer_cancel(SoftTimerId timer_id) {
  const bool critical = critical_section_start();
  if (timer_id < SOFT_TIMER_MAX_TIMERS && s_posix_timers[timer_id].inuse) {
    // Clear the timer if it is in use by setting it_val to 0, 0.
    struct itimerspec spec = { { 0, 0 }, { 0, 0 } };
    timer_settime(s_posix_timers[timer_id].timer_id, 0, &spec, NULL);
    s_posix_timers[timer_id].inuse = false;
    s_active_timers--;
    critical_section_end(critical);
    return true;
  }
  critical_section_end(critical);
  return false;
}

uint32_t soft_timer_remaining_time(SoftTimerId timer_id) {
  struct itimerspec spec = { { 0, 0 }, { 0, 0 } };
  timer_gettime(s_posix_timers[timer_id].timer_id, &spec);
  return spec.it_value.tv_sec * 1000000 + spec.it_value.tv_nsec / 1000;
}
