#include "status.h"

#include <stdbool.h>
#include <stdio.h>

#include "misc.h"
#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {}

static StatusCode prv_with_msg() {
  return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "my-message");
}

// Verifies most common use cases.
void test_status_create_valid(void) {
  // Most basic use case.
  StatusCode ok = status_code(STATUS_CODE_UNIMPLEMENTED);
  if (!ok) {
    Status status = status_get();
    TEST_ASSERT_EQUAL(STATUS_CODE_UNIMPLEMENTED, status.code);
    TEST_ASSERT_EQUAL_STRING("test_status_create_valid", status.caller);
    TEST_ASSERT_EQUAL_STRING("", status.message);
  }

  // Most basic use case with a message.
  ok = status_msg(STATUS_CODE_UNKNOWN, "error");
  if (!ok) {
    Status status = status_get();
    TEST_ASSERT_EQUAL(STATUS_CODE_UNKNOWN, status.code);
    TEST_ASSERT_EQUAL_STRING("test_status_create_valid", status.caller);
    TEST_ASSERT_EQUAL_STRING("error", status.message);
  }

  // Slightly indirect use case.
  ok = prv_with_msg();
  if (!ok) {
    Status status = status_get();
    TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, status.code);
    TEST_ASSERT_EQUAL_STRING("prv_with_msg", status.caller);
    TEST_ASSERT_EQUAL_STRING("my-message", status.message);
  }
}

// Verifies the behavior when a code greater than NUM_STATUS_CODE is entered.
void test_status_create_invalid_code(void) {
  StatusCode statuscode = status_code(NUM_STATUS_CODES + 1);
  Status status = status_get();
  TEST_ASSERT_EQUAL(NUM_STATUS_CODES + 1, status.code);
  TEST_ASSERT_EQUAL_STRING("test_status_create_invalid_code", status.caller);
  TEST_ASSERT_EQUAL_STRING("", status.message);
}

static StatusCode prv_ok_or_return() {
  status_ok_or_return(status_code(STATUS_CODE_OK));
  status_ok_or_return(status_msg(STATUS_CODE_TIMEOUT, "This should work."));
  return status_code(STATUS_CODE_UNKNOWN);
}

// Increments an external count to check how many times the function is called
static StatusCode prv_status_num_calls(uint16_t *count) {
  (*count)++;
  // Returning an error to check that the function isn't being called twice on failure
  return STATUS_CODE_INTERNAL_ERROR;
}

static StatusCode prv_ok_or_return_function_count(uint16_t *count) {
  status_ok_or_return(prv_status_num_calls(count));
  return STATUS_CODE_OK;
}

// Tests the ok_or_return macro
void test_status_ok_or_return(void) {
  // Track the number of calls the test function makes
  uint16_t num_calls = 0;
  StatusCode ok = prv_ok_or_return();
  Status status = status_get();
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, status.code);
  TEST_ASSERT_EQUAL_STRING("prv_ok_or_return", status.caller);
  TEST_ASSERT_EQUAL_STRING("This should work.", status.message);
  // Checks that the function passed in to the macro is only called once
  prv_ok_or_return_function_count(&num_calls);
  TEST_ASSERT_EQUAL(1, num_calls);
}

void test_status_clear(void) {
  StatusCode ok = status_code(STATUS_CODE_UNIMPLEMENTED);
  status_clear();
  Status status = status_get();
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, status.code);
  TEST_ASSERT_EQUAL_STRING("test_status_clear", status.caller);
  TEST_ASSERT_EQUAL_STRING("Clear", status.message);
}

static bool s_foo = false;

static void prv_test_callback(const Status *status) {
  s_foo = true;
  printf("CODE:%d:%s:%s: %s\n", status->code, status->source, status->caller, status->message);
}

void test_status_register_callback(void) {
  status_register_callback(prv_test_callback);
  TEST_ASSERT_FALSE(s_foo);
  status_msg(STATUS_CODE_EMPTY, "This is cool!");
  TEST_ASSERT_TRUE(s_foo);
}
