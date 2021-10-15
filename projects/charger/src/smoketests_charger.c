#include "smoketests_charger.h"

#include "log.h"
#include "smoke_charger_comms.h"

// Smoke test perform functions must obey this signature.
typedef void (*SmokeTestFunction)(void);

// Add a line to this lookup table to add a smoke test.
static SmokeTestFunction s_smoke_tests[NUM_SMOKE_TESTS] = {
  [SMOKE_TEST_CHARGER_COMMS] = smoke_current_measurement_perform,
};

void smoketests_charger_run(SmokeTest smoke_test, const char *smoke_test_name) {
  if (smoke_test >= NUM_SMOKE_TESTS) {
    LOG_CRITICAL(
        "Invalid smoke test! Please set CHARGER_SMOKE_TEST to a valid value from the SmokeTest enum, "
        "or comment it out to run Charger normally. (Name=%s, value=%d)\n",
        smoke_test_name, smoke_test);
    return;
  }
  if (s_smoke_tests[smoke_test] == NULL) {
    LOG_CRITICAL(
        "Smoke test '%s' (%d) has no entry in lookup table! Please add it to s_smoke_tests "
        "in smoke_tests_charger.c\n",
        smoke_test_name, smoke_test);
    return;
  }

  LOG_DEBUG("Running %s\n", smoke_test_name);
  s_smoke_tests[smoke_test]();
}
