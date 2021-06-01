#include "ms_test_helpers.h"
#include <string.h>
#include <time.h>
#include "log.h"
#include "ms_test_helper_can.h"
#include "status.h"

#define CLOCK_HEADSTART 30000
#define TOLERANCE_MS 10

StatusCode ms_test_helpers_time_assert(uint8_t value_to_compare, volatile uint8_t *value_to_test,
                                       double time_ms) {
  double elapsed = 0;
  double upper_bound = (double)(time_ms + TOLERANCE_MS) / 1000.0;
  double lower_bound = (double)(time_ms - TOLERANCE_MS) / 1000.0;
  clock_t begin = clock() - CLOCK_HEADSTART;
  while (elapsed < upper_bound) {
    elapsed = ((double)(clock() - begin)) / (CLOCKS_PER_SEC);
    if (value_to_compare == *value_to_test && elapsed > lower_bound) {
      return STATUS_CODE_OK;
    }
  }
  return STATUS_CODE_TIMEOUT;
}
