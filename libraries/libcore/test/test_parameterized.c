#include "log.h"
#include "test_helpers.h"
#include "unity.h"

// Example of parameterized tests - use TEST_CASE with arguments to be passed literally to the
// test function. See https://uwmidsun.atlassian.net/l/c/1yZudbzD.

void setup_test(void) {}
void teardown_test(void) {}

TEST_CASE(1, 2)
TEST_CASE(5, 10)
TEST_CASE(15, 30)
void test_parameterized_ints(uint8_t a, uint8_t b) {
  LOG_DEBUG("a=%d, b=%d\n", a, b);
  TEST_ASSERT_EQUAL(b, 2 * a);
}
