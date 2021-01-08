#include "adc.h"
#include "voltage_regulator.h"

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CALLBACK_EXPECTED_TIMES_CALLED 1
#define TEST_CALLBACK_EXPECTED_CONTEXT 0xA

static const GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 0 };
static const GpioAddress test_monitor_pin = { .port = GPIO_PORT_A, .pin = 1 };
static uint8_t s_regulator_callback_context;
static uint8_t s_regulator_callback_called;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  s_regulator_callback_context = 0xA;
}
static void prv_test_voltage_regulator_interrupt_callback(void *context,
                                                          VoltageRegulatorError error) {
  LOG_DEBUG("FANFAIL INTERRUPT CALLBACK TRIGGERED\n");

  uint8_t *regulator_context = context;
  TEST_ASSERT_EQUAL(TEST_CALLBACK_EXPECTED_CONTEXT, *regulator_context);

  s_regulator_callback_called++;
}

void test_voltage_regulator_init_works(void) {
  VoltageRegulatorSettings settings = { enable_pin = test_enable_pin;
  monitor_pin = test_monitor_pin;
  error_callback = &prv_test_voltage_regulator_interrupt_callback;
  error_callback_context = s_regulator_callback_context;
}
VoltageRegulatorStorage storage = { regulator_on = false;
enable_pin = NULL;
monitor_pin = NULL;
error_callback = NULL;
error_callback_context = NULL;
}

TEST_ASSERT_OK(voltage_regulator_init(&settings, &storage));
gpio_it_trigger_interrupt(&test_monitor_pin);
TEST_ASSERT_EQUAL(TEST_CALLBACK_EXPECTED_TIMES_CALLED, s_regulator_callback_called);
}
