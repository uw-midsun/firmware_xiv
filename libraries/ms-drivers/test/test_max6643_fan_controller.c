#include "interrupt.h"
#include "log.h"
#include "max6643_fan_controller.h"
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"

typedef struct Max6643MockStatus {
  uint8_t FANFAIL_INTERRUPT_STATUS;  // 0 if not triggered, 1 otherwise
  uint8_t OVERTEMP_INTERRUPT_STATUS;
} Max6643MockStatus;

static Max6643MockStatus s_mock_statuses;
static Max6643Storage s_mock_storage;
static Max6643Storage s_storage;

static void prv_test_max6643_fanfail_interrupt_callback(const GpioAddress *address, void *context) {
  LOG_DEBUG("FANFAIL INTERRUPT CALLBACK TRIGGERED\n");

  TEST_ASSERT_EQUAL(1, s_mock_statuses.FANFAIL_INTERRUPT_STATUS);
}

static void prv_test_max6643_overtemp_interrupt_callback(const GpioAddress *address,
                                                         void *context) {
  LOG_DEBUG("OVERTEMP INTERRUPT CALLBACK TRIGGERED\n");

  TEST_ASSERT_EQUAL(1, s_mock_statuses.OVERTEMP_INTERRUPT_STATUS);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  s_mock_statuses.FANFAIL_INTERRUPT_STATUS = 0;
  s_mock_statuses.OVERTEMP_INTERRUPT_STATUS = 0;
}

void teardown_test(void) {}

// Test initializing with valid parameters
void test_max6643_init_works(void) {
  GpioAddress test_fanfail_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_overtemp_pin = { .port = GPIO_PORT_A, .pin = 1 };

  Max6643Settings valid_settings = { .fanfail_pin = test_fanfail_pin,
                                     .overtemp_pin = test_overtemp_pin,
                                     .fanfail_callback = NULL,
                                     .overtemp_callback = NULL,
                                     .fanfail_callback_context = NULL,
                                     .overtemp_callback_context = NULL };

  s_mock_storage.fanfail_pin = valid_settings.fanfail_pin;
  s_mock_storage.overtemp_pin = valid_settings.overtemp_pin;
  s_mock_storage.fanfail_callback = valid_settings.fanfail_callback;
  s_mock_storage.overtemp_callback = valid_settings.overtemp_callback;
  s_mock_storage.fanfail_callback_context = valid_settings.fanfail_callback_context;
  s_mock_storage.overtemp_callback_context = valid_settings.overtemp_callback_context;

  TEST_ASSERT_OK(max6643_init(&s_storage, &valid_settings));

  TEST_ASSERT_EQUAL(s_mock_storage.fanfail_pin.pin, s_storage.fanfail_pin.pin);
  TEST_ASSERT_EQUAL(s_mock_storage.fanfail_pin.port, s_storage.fanfail_pin.port);
  TEST_ASSERT_EQUAL(s_mock_storage.overtemp_pin.pin, s_storage.overtemp_pin.pin);
  TEST_ASSERT_EQUAL(s_mock_storage.overtemp_pin.port, s_storage.overtemp_pin.port);
  TEST_ASSERT_EQUAL(s_mock_storage.fanfail_callback, s_storage.fanfail_callback);
  TEST_ASSERT_EQUAL(s_mock_storage.fanfail_callback_context, s_storage.fanfail_callback_context);
  TEST_ASSERT_EQUAL(s_mock_storage.overtemp_callback, s_storage.overtemp_callback);
  TEST_ASSERT_EQUAL(s_mock_storage.overtemp_callback_context, s_storage.overtemp_callback_context);
}

// test that interrupts work
void test_max6643_interrupts(void) {
  GpioAddress test_fanfail_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_overtemp_pin = { .port = GPIO_PORT_A, .pin = 1 };

  Max6643Settings valid_settings = {
    .fanfail_pin = test_fanfail_pin,
    .overtemp_pin = test_overtemp_pin,
    .fanfail_callback = (GpioItCallback)prv_test_max6643_fanfail_interrupt_callback,
    .overtemp_callback = (GpioItCallback)prv_test_max6643_overtemp_interrupt_callback,
    .fanfail_callback_context = NULL,
    .overtemp_callback_context = NULL
  };

  s_mock_storage.fanfail_pin = valid_settings.fanfail_pin;
  s_mock_storage.overtemp_pin = valid_settings.overtemp_pin;
  s_mock_storage.fanfail_callback = valid_settings.fanfail_callback;
  s_mock_storage.overtemp_callback = valid_settings.overtemp_callback;

  // add callback to storage
  TEST_ASSERT_OK(max6643_init(&s_storage, &valid_settings));

  // trigger interrupt and fetch data
  s_mock_statuses.FANFAIL_INTERRUPT_STATUS = 1;
  gpio_it_trigger_interrupt(&test_fanfail_pin);

  s_mock_statuses.OVERTEMP_INTERRUPT_STATUS = 1;
  gpio_it_trigger_interrupt(&test_overtemp_pin);
}
