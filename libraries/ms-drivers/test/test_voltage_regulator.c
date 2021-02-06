#include "adc.h"
#include "voltage_regulator.h"

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CALLBACK_EXPECTED_TIMES_CALLED 1
#define TEST_CALLBACK_EXPECTED_CONTEXT 0xA

static const GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 0 };
static const GpioAddress test_monitor_pin = { .port = GPIO_PORT_A, .pin = 1 };
static uint8_t s_regulator_callback_context;
static uint8_t s_regulator_callback_called;

VoltageRegulatorStorage storage = { 0 };

StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *input_state) {
  *input_state = GPIO_STATE_HIGH;
  return STATUS_CODE_OK;
}

static void prv_test_voltage_regulator_callback(void *context, VoltageRegulatorError error) {
  LOG_DEBUG("VOLTAGE REGULATOR ERROR CALLBACK TRIGGERED\n");

  uint8_t *regulator_context = context;
  TEST_ASSERT_EQUAL(TEST_CALLBACK_EXPECTED_CONTEXT, *regulator_context);

  s_regulator_callback_called++;
  TEST_ASSERT_EQUAL(TEST_CALLBACK_EXPECTED_TIMES_CALLED, s_regulator_callback_called);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  s_regulator_callback_context = 0xA;
  s_regulator_callback_called = 0;
}

void teardown_test(void) {
  voltage_regulator_stop(&storage);
}

void test_voltage_regulator_init_works(void) {
  VoltageRegulatorSettings settings = { .enable_pin = test_enable_pin,
                                        .monitor_pin = test_monitor_pin,
                                        .error_callback = prv_test_voltage_regulator_callback,
                                        .error_callback_context = &s_regulator_callback_context };

  TEST_ASSERT_OK(voltage_regulator_init(&settings, &storage));
}

void test_voltage_regulator_set_enabled_works(void) {
  VoltageRegulatorSettings settings = { .enable_pin = test_enable_pin,
                                        .monitor_pin = test_monitor_pin,
                                        .error_callback = &prv_test_voltage_regulator_callback,
                                        .error_callback_context = &s_regulator_callback_context };
  voltage_regulator_init(&settings, &storage);
  TEST_ASSERT_OK(voltage_regulator_set_enabled(&storage, true));
  delay_s(2.1);
  TEST_ASSERT_EQUAL(TEST_CALLBACK_EXPECTED_TIMES_CALLED, s_regulator_callback_called);
}
