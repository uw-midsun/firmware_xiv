#include "bts_7200_current_sense.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_I2C_PORT I2C_PORT_2

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

static volatile uint16_t times_callback_called = 0;
static void *received_context;

static void prv_callback_increment(uint16_t reading0, uint16_t reading1, void *context) {
  times_callback_called++;
  received_context = context;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,         //
    .sda = TEST_CONFIG_PIN_I2C_SDA,  //
    .scl = TEST_CONFIG_PIN_I2C_SCL,  //
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);

  times_callback_called = 0;
}
void teardown_test(void) {}

// Comprehensive happy-path test for STM32 initialization.
void test_bts_7200_current_sense_timer_stm32_works(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_OK(bts_7200_init_stm32(&storage, &settings));

  // make sure we don't start anything in init
  TEST_ASSERT_EQUAL(times_callback_called, 0);
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 0);

  TEST_ASSERT_OK(bts_7200_start(&storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 1);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 2);

  TEST_ASSERT_EQUAL(true, bts_7200_stop(&storage));

  // make sure that stop actually stops it
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 2);
}

// Same, but for pca9539r initialization.
void test_bts_7200_current_sense_timer_pca9539r_works(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  Pca9539rGpioAddress test_select_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_OK(bts_7200_init_pca9539r(&storage, &settings));

  // make sure we don't start anything in init
  TEST_ASSERT_EQUAL(times_callback_called, 0);
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 0);

  TEST_ASSERT_OK(bts_7200_start(&storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 1);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 2);

  TEST_ASSERT_EQUAL(true, bts_7200_stop(&storage));

  // make sure that stop actually stops it
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 2);
}

// Test that we can init, start, stop, and start again and it works.
// Essentially the previous test done twice.
void test_bts_7200_current_sense_restart(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_OK(bts_7200_init_stm32(&storage, &settings));
  TEST_ASSERT_OK(bts_7200_start(&storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 1);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 2);

  TEST_ASSERT_EQUAL(true, bts_7200_stop(&storage));

  // make sure it's stopped
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 2);

  // start again
  TEST_ASSERT_OK(bts_7200_start(&storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 3);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 3) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 4);

  TEST_ASSERT_EQUAL(true, bts_7200_stop(&storage));

  // make sure it's stopped
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 4);
}

// Test init failure when the settings are invalid for stm32.
void test_bts_7200_current_sense_stm32_init_invalid_settings(void) {
  // start with invalid select pin
  GpioAddress select_pin = { .port = NUM_GPIO_PORTS, .pin = 0 };  // invalid
  GpioAddress sense_pin = { .port = 0, .pin = 0 };                // valid
  Bts7200Stm32Settings settings = {
    .select_pin = &select_pin,
    .sense_pin = &sense_pin,
    .interval_us = 500,  // 0.5 ms
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_NOT_OK(bts_7200_init_stm32(&storage, &settings));

  // invalid sense pin
  select_pin.port = 0;
  sense_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(bts_7200_init_stm32(&storage, &settings));

  // otherwise valid
  sense_pin.port = 0;
  TEST_ASSERT_OK(bts_7200_init_stm32(&storage, &settings));
}

// Same, but for pca9539r.
void test_bts_7200_current_sense_pca9539r_init_invalid_settings(void) {
  // start with invalid select pin
  Pca9539rGpioAddress select_pin = { .i2c_address = 0, .pin = NUM_PCA9539R_GPIO_PINS };  // invalid
  GpioAddress sense_pin = { .port = GPIO_PORT_A, .pin = 0 };                             // valid
  Bts7200Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .select_pin = &select_pin,
    .sense_pin = &sense_pin,
    .interval_us = 500,  // 0.5 ms
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_NOT_OK(bts_7200_init_pca9539r(&storage, &settings));

  // invalid sense pin
  select_pin.pin = 0;
  sense_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(bts_7200_init_pca9539r(&storage, &settings));

  // otherwise valid
  sense_pin.port = GPIO_PORT_A;
  TEST_ASSERT_OK(bts_7200_init_pca9539r(&storage, &settings));
}

// Test that having a NULL callback works and we don't segfault.
void test_bts_7200_current_sense_null_callback(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = NULL,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_OK(bts_7200_init_stm32(&storage, &settings));
  TEST_ASSERT_OK(bts_7200_start(&storage));
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&storage));
}

// Test that bts_7200_get_measurement returns ok.
void test_bts_7200_current_sense_get_measurement_stm32_valid(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_OK(bts_7200_init_stm32(&storage, &settings));

  uint16_t reading0 = 0, reading1 = 0;
  TEST_ASSERT_OK(bts_7200_get_measurement(&storage, &reading0, &reading1));
  LOG_DEBUG("STM32 readings: %d, %d\r\n", reading0, reading1);
}

// Same, but with pca9539r initialization.
void test_bts_7200_current_sense_get_measurement_pca9539r_valid(void) {
  Pca9539rGpioAddress test_select_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_OK(bts_7200_init_pca9539r(&storage, &settings));

  uint16_t reading0 = 0, reading1 = 0;
  TEST_ASSERT_OK(bts_7200_get_measurement(&storage, &reading0, &reading1));
  LOG_DEBUG("PCA9539R readings: %d, %d\r\n", reading0, reading1);
}

// Test that bts_7200_stop returns true only when it stops a timer
void test_bts_7200_current_sense_stop_return_behaviour(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_OK(bts_7200_init_stm32(&storage, &settings));
  TEST_ASSERT_EQUAL(false, bts_7200_stop(&storage));
  TEST_ASSERT_OK(bts_7200_start(&storage));
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&storage));
  TEST_ASSERT_EQUAL(false, bts_7200_stop(&storage));
}

// Test that the context is actually passed to the function
void test_bts_7200_current_sense_context_passed(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  uint32_t interval_us = 500;            // 0.5 ms
  void *context_pointer = &interval_us;  // arbitrary pointer
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .callback_context = context_pointer,
  };
  Bts7200Storage storage = { 0 };

  TEST_ASSERT_OK(bts_7200_init_stm32(&storage, &settings));
  TEST_ASSERT_OK(bts_7200_start(&storage));
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&storage));
  TEST_ASSERT_EQUAL(received_context, context_pointer);
}
