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

  // Convert t to nanoseconds
  time_t end = t * 1000;

  // Start clock
  clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
  time_t start_sec = timer.tv_sec;
  time_t start_nano = timer.tv_nsec;

  while ((((timer.tv_sec - start_sec) * 1000000000) + (double)(timer.tv_nsec - start_nano)) < end) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &timer);
  }
}
