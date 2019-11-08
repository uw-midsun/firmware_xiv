#include "pwm_input.h"

#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "pwm.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// To run this test, connect the input and output pins

#define TEST_OUTPUT_PWM_TIMER PWM_TIMER_1
#define TEST_OUTPUT_PWM_ALTFN GPIO_ALTFN_2
#define TEST_OUTPUT_PWM_PERIOD_US 1000
#define TEST_OUTPUT_PWM_ADDR \
  { .port = GPIO_PORT_A, .pin = 8, }

#define TEST_INPUT_PWM_TIMER PWM_TIMER_3
#define TEST_INPUT_PWM_ALTFN GPIO_ALTFN_1
#define TEST_INPUT_PWM_CHANNEL PWM_CHANNEL_2
#define TEST_INPUT_PWM_ADDR \
  { .port = GPIO_PORT_B, .pin = 5, }

#define TOLERANCE (2)

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
}

void teardown_test(void) {}

void print_reading(uint32_t dc, PwmInputReading *reading) {
  LOG_DEBUG("DC: %d, read DC: %d | Period: %d, read period: %d\n", (int)dc,
            (int)reading->dc_percent, TEST_OUTPUT_PWM_PERIOD_US, (int)reading->period_us);
}

void check_pwm_value(uint32_t dc, PwmInputReading *reading) {
  TEST_ASSERT_OK(pwm_set_dc(TEST_OUTPUT_PWM_TIMER, dc));
  delay_ms(50);
  TEST_ASSERT_OK(pwm_input_get_reading(TEST_INPUT_PWM_TIMER, reading));
  print_reading(dc, reading);

  // Workaround for known caveat. Please see pwm input header file for
  // description
  if (dc == 0) {
    TEST_ASSERT_OK(pwm_input_get_reading(TEST_INPUT_PWM_TIMER, reading));
    print_reading(dc, reading);
    TEST_ASSERT_EQUAL(0, reading->dc_percent);
    TEST_ASSERT_EQUAL(0, reading->period_us);
  } else {
    TEST_ASSERT_TRUE((uint32_t)dc * 10 + TOLERANCE > reading->dc_percent);
    TEST_ASSERT_TRUE(((uint32_t)dc == 0 ? (uint32_t)0 : (uint32_t)dc * 10 - TOLERANCE) <=
                     reading->dc_percent);

    TEST_ASSERT_TRUE((uint32_t)TEST_OUTPUT_PWM_PERIOD_US - TOLERANCE < reading->period_us);
    TEST_ASSERT_TRUE((uint32_t)TEST_OUTPUT_PWM_PERIOD_US + TOLERANCE > reading->period_us);
  }
}

void test_pwm_input(void) {
  PwmTimer input_timer = TEST_INPUT_PWM_TIMER;
  PwmTimer output_timer = TEST_OUTPUT_PWM_TIMER;

  GpioAddress input = TEST_INPUT_PWM_ADDR;

  GpioSettings input_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = TEST_INPUT_PWM_ALTFN,
  };

  GpioAddress output = TEST_OUTPUT_PWM_ADDR;

  GpioSettings output_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = TEST_OUTPUT_PWM_ALTFN,
  };

  TEST_ASSERT_OK(gpio_init_pin(&output, &output_settings));
  TEST_ASSERT_OK(gpio_init_pin(&input, &input_settings));

  TEST_ASSERT_OK(pwm_init(TEST_OUTPUT_PWM_TIMER, TEST_OUTPUT_PWM_PERIOD_US));

  TEST_ASSERT_OK(pwm_input_init(input_timer, TEST_INPUT_PWM_CHANNEL));

  PwmInputReading reading = { 0 };

  for (uint32_t b = 0; b < 3; b++) {
    for (uint32_t i = 0; i <= 100; i++) {
      check_pwm_value(i, &reading);
    }
  }
}
