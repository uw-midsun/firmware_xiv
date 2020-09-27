
#pragma once

// Created by Sudhish M on 2020-09-25.
// usingSoftTimers

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_PERIOD_MS 500

typedef struct counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  // &counterTest=counterPtr
  Counters *counterPtr = context;

  counterPtr->counter_a++;
  LOG_DEBUG("Counter A: %i \n", counterPtr->counter_a);

  if (counterPtr->counter_a % 2 == 0) {
    counterPtr->counter_b++;
    LOG_DEBUG("Counter B: %i \n", counterPtr->counter_b);
  }

  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_timer_callback, counterPtr, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counterTest = { 0 };

  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_timer_callback, &counterTest, NULL);

  while (true) {
    wait();
  }

  return 0;
}
