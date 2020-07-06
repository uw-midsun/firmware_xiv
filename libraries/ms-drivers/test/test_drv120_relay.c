// Brief test sequence for DRV120

#include "drv120_relay.h"
#include "log.h"
#include "ms_test_helpers.h"

// Pin used to enable/disable relay
const GpioAddress TEST_DRV120_PIN = { GPIO_PORT_A, 8 };

void setup_test(void) {
  gpio_init();
}

void teardown_test(void) {}

void test_drv120_relay(void) {
  // Variable used later to get GPIO state
  GpioState state = GPIO_STATE_LOW;

  LOG_DEBUG("Initializing relay \n");
  TEST_ASSERT_OK(drv120_relay_init(&TEST_DRV120_PIN));

  // Port should initialize to low: relay closed
  gpio_get_state(&TEST_DRV120_PIN, &state);
  TEST_ASSERT_EQUAL(state, GPIO_STATE_HIGH);

  // Test opening relay
  LOG_DEBUG("Opening relay \n");
  TEST_ASSERT_OK(drv120_relay_open());
  gpio_get_state(&TEST_DRV120_PIN, &state);
  TEST_ASSERT_EQUAL(state, GPIO_STATE_LOW);

  // Test closing relay again
  LOG_DEBUG("Closing relay \n");
  TEST_ASSERT_OK(drv120_relay_close());
  gpio_get_state(&TEST_DRV120_PIN, &state);
  TEST_ASSERT_EQUAL(state, GPIO_STATE_HIGH);
}
