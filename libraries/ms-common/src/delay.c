#include "delay.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "soft_timer.h"
#include "wait.h"

void delay_us(uint32_t t) {
  volatile struct timespec start, timer;

  // Start clocks
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  clock_gettime(CLOCK_MONOTONIC_RAW, &timer);

  // Convert t to nanoseconds
  uint32_t end = t * 1000;

  while ((((timer.tv_sec - start.tv_sec) * 1000000000) + (double)(timer.tv_nsec - start.tv_nsec)) <
         end) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
  }
}
