#include "delay.h"

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#include "soft_timer.h"
#include "wait.h"

void delay_us(uint32_t t) {
  clock_t end = t * (CLOCKS_PER_SEC / 1000000);
  clock_t start = clock();

  while ((clock() - start) < end) {
  }
}
