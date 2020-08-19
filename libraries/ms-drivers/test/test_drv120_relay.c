// Brief test sequence for DRV120

#include "drv120_relay.h"
#include "log.h"
#include "ms_test_helpers.h"

// Pin used to enable/disable relay
static const GpioAddress s_test_drv120_pin = { GPIO_PORT_A, 8 };

void setup_test(void) {
  LOG_DEBUG("Initializing GPIO\n");
  TEST_ASSERT_OK(gpio_init());
}

void teardown_test(void) {}

void test_drv120_relay(void) {
  // Variable used later to get GPIO state
  GpioState state = NUM_GPIO_STATES;
  bool closed;

  LOG_DEBUG("Initializing relay\n");
  TEST_ASSERT_OK(drv120_relay_init(&s_test_drv120_pin));

  // Pin should initialize to low: relay closed
  gpio_get_state(&s_test_drv120_pin, &state);
  TEST_ASSERT_EQUAL(state, GPIO_STATE_LOW);
  TEST_ASSERT_OK(drv120_relay_get_is_closed(&closed));
  TEST_ASSERT_FALSE(closed);

  // Test closing relay
  LOG_DEBUG("Closing relay\n");
  TEST_ASSERT_OK(drv120_relay_close());
  gpio_get_state(&s_test_drv120_pin, &state);
  TEST_ASSERT_EQUAL(state, GPIO_STATE_HIGH);
  TEST_ASSERT_OK(drv120_relay_get_is_closed(&closed));
  TEST_ASSERT_TRUE(closed);

  // Test opening relay again
  LOG_DEBUG("Opening relay\n");
  TEST_ASSERT_OK(drv120_relay_open());
  gpio_get_state(&s_test_drv120_pin, &state);
  TEST_ASSERT_EQUAL(state, GPIO_STATE_LOW);
  TEST_ASSERT_OK(drv120_relay_get_is_closed(&closed));
  TEST_ASSERT_FALSE(closed);
}
