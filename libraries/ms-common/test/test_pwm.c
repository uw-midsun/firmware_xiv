#include "pwm.h"

#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_PWM_TIMER PWM_TIMER_14
#define TEST_PWM_ADDR \
  { GPIO_PORT_A, 7 }
#define TEST_PWM_ALTFN GPIO_ALTFN_4

#define TEST_PWM_PERIOD_MS 100
#define TEST_PWM_DUTY_CYCLE 50
#define TEST_PWM_EXPECTED_EDGES 3

static volatile uint16_t s_counter = 0;

static void prv_pwm_test(const GpioAddress *address, void *context) {
  s_counter++;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  s_counter = 0;
}

void teardown_test(void) {}

void test_pwm_guards(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, pwm_set_pulse(TEST_PWM_TIMER, TEST_PWM_DUTY_CYCLE));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_init(TEST_PWM_TIMER, 0));
  TEST_ASSERT_OK(pwm_init(TEST_PWM_TIMER, TEST_PWM_PERIOD_MS));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_set_dc(TEST_PWM_TIMER, 101));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    pwm_set_pulse(TEST_PWM_TIMER, TEST_PWM_PERIOD_MS + 1));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_init(NUM_PWM_TIMERS, TEST_PWM_PERIOD_MS));
}

void test_pwm_output(void) {
  TEST_ASSERT_OK(pwm_init(TEST_PWM_TIMER, TEST_PWM_PERIOD_MS));
  TEST_ASSERT_EQUAL(TEST_PWM_PERIOD_MS, pwm_get_period(TEST_PWM_TIMER));
  TEST_ASSERT_OK(pwm_set_pulse(TEST_PWM_TIMER, TEST_PWM_PERIOD_MS - 1));
  TEST_ASSERT_OK(pwm_set_dc(TEST_PWM_TIMER, TEST_PWM_DUTY_CYCLE));

  const GpioAddress addr = TEST_PWM_ADDR;
  const GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = TEST_PWM_ALTFN,
  };
  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };
  TEST_ASSERT_OK(gpio_init_pin(&addr, &settings));
  TEST_ASSERT_OK(
      gpio_it_register_interrupt(&addr, &it_settings, INTERRUPT_EDGE_RISING, prv_pwm_test, NULL));
  delay_ms((TEST_PWM_EXPECTED_EDGES + 1) * TEST_PWM_PERIOD_MS);
  TEST_ASSERT_TRUE(s_counter >= TEST_PWM_EXPECTED_EDGES);
}
