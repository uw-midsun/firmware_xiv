#include "bts_7200_current_sense.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

static volatile uint16_t times_callback_called = 0;

static void prv_callback_increment(void) {
  times_callback_called++;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);
  times_callback_called = 0;
}
void teardown_test(void) {}

void test_bts_7200_current_sense_timer_works(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  bts_7200_init(&settings, &storage);

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 1);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 2);

  bts_7200_cancel(&storage);

  // make sure that cancel actually stops it
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 2);
}
