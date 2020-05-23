#pragma once
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Callback function for counter 1
static void counter_a_timeout(SoftTimerId timer_id, void *context);

// Callback function for counter 2
static void counter_b_timeout(SoftTimerId timer_id, void *context);
