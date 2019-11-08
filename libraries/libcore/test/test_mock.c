#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

// Example of a mock - add function name to $(T)_test_[name]_MOCKS to override
// behavior

StatusCode TEST_MOCK(status_impl_update)(StatusCode code, const char *source, const char *caller,
                                         const char *message) {
  LOG_DEBUG("Mock: code %d\nsrc: %s\ncaller: %s\nmsg: %s\n", code, source, caller, message);

  return STATUS_CODE_OK;
}

void setup_test(void) {}

void teardown_test(void) {}

void test_mock_usage(void) {
  status_msg(STATUS_CODE_INTERNAL_ERROR, "this is a message\n");
}
