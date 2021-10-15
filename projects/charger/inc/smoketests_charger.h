#pragma once

#include "misc.h"

// Smoke test control for Charger.

// To run a smoke test, uncomment this and set SMOKE_TEST_CHARGER to the smoke test you want to run.
// #define CHARGER_SMOKE_TEST SMOKE_TEST_CHARGER_COMMS

typedef enum {
  SMOKE_TEST_CHARGER_COMMS = 0,  // see smoke_charger_comms.c
  NUM_SMOKE_TESTS,
} SmokeTest;

// Run the given smoke test.
#define RUN_SMOKE_TEST(smoke_test) smoketests_charger_run((smoke_test), STRINGIFY(smoke_test))

// For internal use, client code should use RUN_SMOKE_TEST.
void smoketests_charger_run(SmokeTest smoke_test, const char *smoke_test_name);
