#pragma once
// Software-based timers backed by single hardware timer
// Requires interrupts to be initialized.
#include <stdbool.h>
#include <stdint.h>

#include "status.h"

#define SOFT_TIMER_MIN_TIME_US 50
#define SOFT_TIMER_MAX_TIMERS 30

#define SOFT_TIMER_INVALID_TIMER (SOFT_TIMER_MAX_TIMERS)

typedef uint16_t SoftTimerId;

typedef void (*SoftTimerCallback)(SoftTimerId timer_id, void *context);

// Initializes a set of software timers. Clock speed should be in MHz.
// Subsequent calls will do nothing. The clock speed is that of the external PLL
// crystal.
void soft_timer_init(void);

// Adds a software timer. The provided duration is the number of microseconds
// before running and the callback is the process to run once the time has
// expired. The timer_id is set to the id of the timer that will run the
// callback.
StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void *context,
                            SoftTimerId *timer_id);

// Starts a software timer in milliseconds. Max duration is still UINT32_MAX us.
#define soft_timer_start_millis(duration_ms, callback, context, timer_id) \
  soft_timer_start((duration_ms)*1000, (callback), (context), (timer_id))

// Starts a software timer in seconds. Max duration is still UINT32_MAX us.
#define soft_timer_start_seconds(duration_s, callback, context, timer_id) \
  soft_timer_start((duration_s)*1000000, (callback), (context), (timer_id))

// Cancels the soft timer specified by id. Returns true if successful.
bool soft_timer_cancel(SoftTimerId timer_id);

// Checks if software timers are running. Returns true if any soft timers are in
// use.
bool soft_timer_inuse(void);

// Checks the time left on a particular timer. Returns a 0 if the timer has
// expired and is no longer in use, or if timer_id is invalid. Note that since timer ids are re-used
// this could return false values once the timer has expired or if it is cancelled.
uint32_t soft_timer_remaining_time(SoftTimerId timer_id);
