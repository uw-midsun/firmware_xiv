#include "killswitch.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "bms.h"
#include "exported_enums.h"
#include "log.h"
#include "test_helpers.h"

static uint8_t s_bps_fault_mask = 0;
static bool s_bps_fault_clear = false;
static uint8_t s_fault_bps_calls = 0;
static GpioState s_gpio_state_get = NUM_GPIO_STATES;

static DebouncerStorage s_killswitch_storage = { 0 };

StatusCode TEST_MOCK(fault_bps)(uint8_t fault_bitmask, bool clear) {
  s_bps_fault_mask = fault_bitmask;
  s_bps_fault_clear = clear;
  s_fault_bps_calls++;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *state) {
  *state = s_gpio_state_get;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  s_bps_fault_mask = 0;
  s_bps_fault_clear = false;
  s_fault_bps_calls = 0;
  s_gpio_state_get = NUM_GPIO_STATES;
  memset(&s_killswitch_storage, 0, sizeof(s_killswitch_storage));
  gpio_init();
  gpio_it_init();
}

void teardown_test(void) {}

void test_killswitch_init(void) {
  s_gpio_state_get = GPIO_STATE_LOW;
  TEST_ASSERT_OK(killswitch_init(&s_killswitch_storage));
  // assert handler was called
  TEST_ASSERT_EQUAL(1, s_fault_bps_calls);
  TEST_ASSERT_TRUE(s_bps_fault_clear);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_KILLSWITCH, s_bps_fault_mask);
}

void test_handler_fault(void) {
  s_gpio_state_get = GPIO_STATE_HIGH;
  TEST_ASSERT_OK(killswitch_init(&s_killswitch_storage));
  // assert handler was called
  TEST_ASSERT_EQUAL(1, s_fault_bps_calls);
  TEST_ASSERT_FALSE(s_bps_fault_clear);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_KILLSWITCH, s_bps_fault_mask);
}
