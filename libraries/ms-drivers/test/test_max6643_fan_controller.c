#include "max6643_fan_controller.h"

#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"

static uint8_t s_times_fanfail_callback_called;
static uint8_t s_times_overtemp_callback_called;
static uint8_t s_fanfail_callback_context;
static uint8_t s_overtemp_callback_context;
static const GpioAddress test_fanfail_pin = { .port = GPIO_PORT_A, .pin = 0 };
static const GpioAddress test_overtemp_pin = { .port = GPIO_PORT_A, .pin = 1 };

static void prv_test_max6643_fanfail_interrupt_callback(const GpioAddress *address, void *context) {
  LOG_DEBUG("FANFAIL INTERRUPT CALLBACK TRIGGERED\n");

  uint8_t *fanfail_context = (uint8_t *)context;
  TEST_ASSERT_EQUAL(0xA, *fanfail_context);

  s_times_fanfail_callback_called++;
}

static void prv_test_max6643_overtemp_interrupt_callback(const GpioAddress *address,
                                                         void *context) {
  LOG_DEBUG("OVERTEMP INTERRUPT CALLBACK TRIGGERED\n");

  uint8_t *overtemp_context = (uint8_t *)context;
  TEST_ASSERT_EQUAL(0xB, *overtemp_context);

  s_times_overtemp_callback_called++;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  s_times_fanfail_callback_called = 0;
  s_times_overtemp_callback_called = 0;
  s_fanfail_callback_context = 0xA;
  s_overtemp_callback_context = 0xB;
}

void teardown_test(void) {}

// Test initializing with valid parameters
void test_max6643_init_works(void) {
  Max6643Settings valid_settings = { .fanfail_pin = test_fanfail_pin,
                                     .overtemp_pin = test_overtemp_pin,
                                     .fanfail_callback = NULL,
                                     .overtemp_callback = NULL,
                                     .fanfail_callback_context = NULL,
                                     .overtemp_callback_context = NULL };

  TEST_ASSERT_OK(max6643_init(&valid_settings));
}

// test that interrupts work
void test_max6643_interrupts(void) {
  Max6643Settings valid_settings = {
    .fanfail_pin = test_fanfail_pin,
    .overtemp_pin = test_overtemp_pin,
    .fanfail_callback = (GpioItCallback)prv_test_max6643_fanfail_interrupt_callback,
    .overtemp_callback = (GpioItCallback)prv_test_max6643_overtemp_interrupt_callback,
    .fanfail_callback_context = &s_fanfail_callback_context,
    .overtemp_callback_context = &s_overtemp_callback_context
  };

  // add callback to storage
  TEST_ASSERT_OK(max6643_init(&valid_settings));

  // trigger interrupt and fetch data
  gpio_it_trigger_interrupt(&test_fanfail_pin);
  TEST_ASSERT_EQUAL(1, s_times_fanfail_callback_called);

  gpio_it_trigger_interrupt(&test_overtemp_pin);
  TEST_ASSERT_EQUAL(1, s_times_overtemp_callback_called);
}
