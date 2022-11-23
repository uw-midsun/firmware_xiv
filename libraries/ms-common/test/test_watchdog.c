#include "delay.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "watchdog.h"

WatchdogStorage s_watchdog;

#define TIMEOUT_MS 50

static bool s_expiry_called = false;

static void *s_passed_context;

static void prv_expiry_callback(void *context) {
  s_expiry_called = true;
  s_passed_context = context;
}

const WatchdogSettings settings = {
  .timeout_ms = TIMEOUT_MS,
  .callback = prv_expiry_callback,
};

static void prv_reset_callback(void) {
  s_expiry_called = false;
  s_passed_context = (void *)0;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  prv_reset_callback();
}

void teardown_test(void) {}

void test_watchdog_expiry(void) {
  uint32_t context_data = 0xdeadbeef;
  watchdog_start(&s_watchdog, TIMEOUT_MS, prv_expiry_callback, &context_data);
  TEST_ASSERT_FALSE(s_expiry_called);

  delay_ms(TIMEOUT_MS + 50);

  TEST_ASSERT_TRUE(s_expiry_called);
  TEST_ASSERT_EQUAL(&context_data, s_passed_context);
}

void test_watchdog_kick(void) {
  uint32_t context_data = 0xdeadbeef;
  watchdog_start(&s_watchdog, TIMEOUT_MS, prv_expiry_callback, &context_data);

  delay_ms(TIMEOUT_MS - 5);
  TEST_ASSERT_FALSE(s_expiry_called);

  watchdog_kick(&s_watchdog);
  delay_ms(10);
  TEST_ASSERT_FALSE(s_expiry_called);

  delay_ms(TIMEOUT_MS);
  TEST_ASSERT_TRUE(s_expiry_called);
  TEST_ASSERT_EQUAL(&context_data, s_passed_context);
}

void test_watchdog_expired_does_not_call_callback_multiple_times(void) {
  uint32_t context_data = 0xdeadbeef;
  watchdog_start(&s_watchdog, TIMEOUT_MS, prv_expiry_callback, &context_data);

  delay_ms(TIMEOUT_MS + 20);

  TEST_ASSERT_TRUE(s_expiry_called);
  TEST_ASSERT_EQUAL(&context_data, s_passed_context);
  prv_reset_callback();

  delay_ms(TIMEOUT_MS + 5);
  TEST_ASSERT_FALSE(s_expiry_called);
}

void test_watchdog_expired_can_start_again(void) {
  uint32_t context_data = 0xdeadbeef;
  watchdog_start(&s_watchdog, TIMEOUT_MS, prv_expiry_callback, &context_data);

  // its expired
  delay_ms(TIMEOUT_MS + 20);
  TEST_ASSERT_TRUE(s_expiry_called);
  prv_reset_callback();

  // started again
  watchdog_start(&s_watchdog, TIMEOUT_MS, prv_expiry_callback, &context_data);
  delay_ms(TIMEOUT_MS - 5);

  // kicked
  watchdog_kick(&s_watchdog);
  delay_ms(10);

  // not expired cuz kicked
  TEST_ASSERT_FALSE(s_expiry_called);

  // expired again
  delay_ms(TIMEOUT_MS + 5);
  TEST_ASSERT_TRUE(s_expiry_called);
  TEST_ASSERT_EQUAL(&context_data, s_passed_context);
}
