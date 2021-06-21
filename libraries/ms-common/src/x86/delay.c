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
