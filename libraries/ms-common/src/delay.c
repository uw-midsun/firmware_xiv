#include "delay.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "soft_timer.h"
#include "wait.h"

void delay_us(uint32_t t) {
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
