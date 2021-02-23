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
#define TEST_TIMER_CALLBACK_DELAY_MS 100

static const GpioAddress s_test_enable_pin = { .port = GPIO_PORT_A, .pin = 0 };
static const GpioAddress s_test_monitor_pin = { .port = GPIO_PORT_A, .pin = 1 };
static uint8_t s_regulator_callback_context;
static uint8_t s_regulator_callback_called;
static GpioState current_state;

VoltageRegulatorStorage s_storage = { 0 };

StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *input_state) {
  *input_state = current_state;
  return STATUS_CODE_OK;
}

static void prv_test_voltage_regulator_callback(VoltageRegulatorError error, void *context) {
  LOG_DEBUG("VOLTAGE REGULATOR ERROR CALLBACK TRIGGERED\n");

  uint8_t *regulator_context = context;
  TEST_ASSERT_EQUAL(TEST_CALLBACK_EXPECTED_CONTEXT, *regulator_context);
  s_regulator_callback_called++;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  s_regulator_callback_context = 0xA;
  s_regulator_callback_called = 0;
}

void teardown_test(void) {
  voltage_regulator_stop(&s_storage);
}

void test_voltage_regulator_init_works(void) {
  VoltageRegulatorSettings settings = { .enable_pin = s_test_enable_pin,
                                        .monitor_pin = s_test_monitor_pin,
                                        .timer_callback_delay_ms = TEST_TIMER_CALLBACK_DELAY_MS,
                                        .error_callback = prv_test_voltage_regulator_callback,
                                        .error_callback_context = &s_regulator_callback_context };

  TEST_ASSERT_OK(voltage_regulator_init(&s_storage, &settings));
}

void test_voltage_regulator_set_enabled_works_enable_true_regulator_off(void) {
  current_state = GPIO_STATE_LOW;
  VoltageRegulatorSettings settings = { .enable_pin = s_test_enable_pin,
                                        .monitor_pin = s_test_monitor_pin,
                                        .timer_callback_delay_ms = TEST_TIMER_CALLBACK_DELAY_MS,
                                        .error_callback = prv_test_voltage_regulator_callback,
                                        .error_callback_context = &s_regulator_callback_context };
  voltage_regulator_init(&s_storage, &settings);
  TEST_ASSERT_OK(voltage_regulator_set_enabled(&s_storage, true));
  delay_ms(TEST_TIMER_CALLBACK_DELAY_MS + 10);
  TEST_ASSERT_EQUAL(TEST_CALLBACK_EXPECTED_TIMES_CALLED, s_regulator_callback_called);
}

void test_voltage_regulator_set_enabled_works_enable_true_regulator_on(void) {
  current_state = GPIO_STATE_HIGH;
  VoltageRegulatorSettings settings = { .enable_pin = s_test_enable_pin,
                                        .monitor_pin = s_test_monitor_pin,
                                        .timer_callback_delay_ms = TEST_TIMER_CALLBACK_DELAY_MS,
                                        .error_callback = prv_test_voltage_regulator_callback,
                                        .error_callback_context = &s_regulator_callback_context };
  voltage_regulator_init(&s_storage, &settings);

  TEST_ASSERT_OK(voltage_regulator_set_enabled(&s_storage, true));
}

void test_voltage_regulator_set_enabled_works_enable_false_voltage_off(void) {
  current_state = GPIO_STATE_LOW;
  VoltageRegulatorSettings settings = { .enable_pin = s_test_enable_pin,
                                        .monitor_pin = s_test_monitor_pin,
                                        .timer_callback_delay_ms = TEST_TIMER_CALLBACK_DELAY_MS,
                                        .error_callback = prv_test_voltage_regulator_callback,
                                        .error_callback_context = &s_regulator_callback_context };
  voltage_regulator_init(&s_storage, &settings);

  TEST_ASSERT_OK(voltage_regulator_set_enabled(&s_storage, false));
}

void test_voltage_regulator_set_enabled_works_enable_false_regulator_on(void) {
  current_state = GPIO_STATE_HIGH;
  VoltageRegulatorSettings settings = { .enable_pin = s_test_enable_pin,
                                        .monitor_pin = s_test_monitor_pin,
                                        .timer_callback_delay_ms = TEST_TIMER_CALLBACK_DELAY_MS,
                                        .error_callback = prv_test_voltage_regulator_callback,
                                        .error_callback_context = &s_regulator_callback_context };
  voltage_regulator_init(&s_storage, &settings);
  TEST_ASSERT_OK(voltage_regulator_set_enabled(&s_storage, false));
  delay_ms(TEST_TIMER_CALLBACK_DELAY_MS + 10);
  TEST_ASSERT_EQUAL(TEST_CALLBACK_EXPECTED_TIMES_CALLED, s_regulator_callback_called);
}
