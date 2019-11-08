#pragma once
// Test helper functions. These should only ever be called within a file in the
// test folder.

#include <stdint.h>

// For testing soft_timer.h:
// TEST ONLY FUNCTION TO SET TIMER COUNTER FOR SOFT TIMERS UNSAFE TO CALL
// OUTSIDE A TEST.
void _test_soft_timer_set_counter(uint32_t counter_value);
