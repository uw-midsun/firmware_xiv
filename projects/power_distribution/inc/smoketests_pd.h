#pragma once

#include "misc.h"

// Smoke test control for PD.

// To run a smoke test, uncomment this and set PD_SMOKE_TEST to the smoke test you want to run.
#define PD_SMOKE_TEST SMOKE_TEST_CURRENT_MEASUREMENT

typedef enum {
  SMOKE_TEST_CURRENT_MEASUREMENT = 0,  // see smoke_current_measurement.c
  NUM_SMOKE_TESTS,
} SmokeTest;

typedef enum {
  SMOKE_TEST_UV_CUTOFF = 0,  // see smoke_uv_cutoff.c
  NUM_SMOKE_TESTs,
} SmokeTest;

// Run the given smoke test.
#define RUN_SMOKE_TEST(smoke_test) smoketests_pd_run((smoke_test), STRINGIFY(smoke_test))

// For internal use, client code should use RUN_SMOKE_TEST.
void smoketests_pd_run(SmokeTest smoke_test, const char *smoke_test_name);
