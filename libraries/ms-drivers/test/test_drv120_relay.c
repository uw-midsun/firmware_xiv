// Brief test sequence for DRV120

#include "delay.h"
#include "drv120_relay.h"
#include "log.h"
#include "ms_test_helpers.h"

// Gpio_it callback function
static bool s_gpio_it_callback_called = false;
static uint8_t rcv_context;
void prv_callback(void *context) {
  LOG_DEBUG("DRV120 Interrupt Triggered!\n");
  s_gpio_it_callback_called = true;
  if (context) {
    rcv_context = *(int *)context;
  }
}

// Pin used to enable/disable relay
static const GpioAddress s_test_drv120_pin = { GPIO_PORT_A, 8 };
static const GpioAddress s_test_drv120_status = { GPIO_PORT_A, 6 };

void setup_test(void) {
  LOG_DEBUG("Initializing GPIO\n");
  TEST_ASSERT_OK(gpio_init());
  interrupt_init();
  gpio_it_init();
}

void teardown_test(void) {}

void test_drv120_relay(void) {
  // Variable used later to get GPIO state
  GpioState state = NUM_GPIO_STATES;
  bool closed;

  LOG_DEBUG("Initializing relay\n");
  Drv120RelaySettings settings = {
    .enable_pin = &s_test_drv120_pin,
    .status_pin = NULL,
    .error_handler = prv_callback,
    .context = NULL,
  };
  TEST_ASSERT_OK(drv120_relay_init(&settings));

  // Test that no callback registered, as status pin passed is NULL
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, gpio_it_trigger_interrupt(&s_test_drv120_status));

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

void test_callback_registered(void) {
  LOG_DEBUG("Initializing gpio interrupt\n");
  uint8_t test_ctx = 1;
  Drv120RelaySettings settings = {
    .enable_pin = &s_test_drv120_pin,
    .status_pin = &s_test_drv120_status,
    .error_handler = prv_callback,
    .context = &test_ctx,
  };
  TEST_ASSERT_OK(drv120_relay_init(&settings));
  gpio_it_trigger_interrupt(&s_test_drv120_status);
  TEST_ASSERT_TRUE(s_gpio_it_callback_called);
  TEST_ASSERT_EQUAL(1, rcv_context);
}
