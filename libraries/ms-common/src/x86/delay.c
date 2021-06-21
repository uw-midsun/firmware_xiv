#include "delay.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "soft_timer.h"
#include "wait.h"
#include "x86_interrupt.h"

static void prv_delay_us_sys_clock(uint32_t t) {
  volatile struct timespec timer;

  // Start clock
  clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
  time_t start_sec = timer.tv_sec;
  time_t start_nano = timer.tv_nsec;

  time_t end_sec = start_sec + t / 1000000;
  time_t end_nano = start_nano + (t % 1000000) * 1000;

  while (timer.tv_sec < end_sec || (timer.tv_sec == end_sec && timer.tv_nsec < end_nano)) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
  }
}

// static uint32_t prv_get_time_us() {
//   struct timespec timer;
//   clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
//   return (timer.tv_sec * 1000000 + timer.tv_nsec / 1000);
// }

// void delay_us(uint32_t t) {
//   uint32_t current_time = prv_get_time_us();
//   uint32_t end_time = current_time + t;
//   // since t is uint32, there is a max of 1 rollover
//   bool rollover = (end_time < current_time);

//   while (rollover || current_time < end_time) {
//     // update time
//     uint32_t time = prv_get_time_us();
//     if (time < current_time) {  // rollover detection
//       rollover = false;
//     }
//     current_time = time;
//   }
// }

static void prv_delay_it(SoftTimerId timer_id, void *context) {
  volatile bool *block = context;
  *block = false;
}

static void prv_delay_us_soft_timer(uint32_t t) {
  volatile bool block = true;
  soft_timer_start(t, prv_delay_it, (void *)&block, NULL);
  while (block) {
    wait();
  }
}

void delay_us(uint32_t t) {
  // use softtimer if interrupts are not disabled
  // use clock delay if iterrupts are disabled
  if (x86_interrupt_in_handler()) {  // interrupts disabled
    prv_delay_us_sys_clock(t);
  } else {
    prv_delay_us_soft_timer(t);
  }
}
