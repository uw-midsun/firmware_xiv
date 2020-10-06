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

#define ADC_DEFAULT_RETURNED_VOLTAGE_RAW 2500
#define ADC_EXPECTED_OK_VOLTAGE 1000
#define ADC_TEST_FAULT_VOLTAGE 5000

static uint16_t s_adc_measurement_0 = ADC_EXPECTED_OK_VOLTAGE;
static uint16_t s_adc_measurement_1 = ADC_EXPECTED_OK_VOLTAGE;
// To give different measurements between adc readings in bts_7200_get_measurement
static bool s_adc_measurement_number = 0;

static volatile uint16_t times_callback_called = 0;
static void *received_context;

static volatile uint16_t s_times_fault_callback_called = 0;
static void *fault_received_context;

static void prv_callback_increment(uint16_t reading0, uint16_t reading1, void *context) {
  times_callback_called++;
  received_context = context;
}

static void prv_fault_callback_increment(bool fault0, bool fault1, void *context) {
  s_times_fault_callback_called++;
  fault_received_context = context;
}

// Storage is global so we can call bts_7200_stop to stop soft timers and avoid segfaults
static Bts7200Storage s_storage = { 0 };

// Mocks adc_read_raw to allow for changing the reading during testing.
// Note that this removes some of the interrupt functionality of the x86 adc implementation,
// but this shouldn't matter in this case.
StatusCode TEST_MOCK(adc_read_raw)(AdcChannel adc_channel, uint16_t *reading) {
  // Return reading based on pin it should corrsepond to.
  if (s_adc_measurement_number == 0) {
    *reading = s_adc_measurement_0;
  } else {
    *reading = s_adc_measurement_1;
  }

  s_adc_measurement_number = !s_adc_measurement_number;

  return STATUS_CODE_OK;
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
  s_times_fault_callback_called = 0;

  s_adc_measurement_0 = ADC_EXPECTED_OK_VOLTAGE;
  s_adc_measurement_1 = ADC_EXPECTED_OK_VOLTAGE;
}

void teardown_test(void) {
  bts_7200_stop(&s_storage);
}

// Comprehensive happy-path test for STM32 initialization.
void test_bts_7200_current_sense_timer_stm32_works(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };

  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));

  // make sure we don't start anything in init
  TEST_ASSERT_EQUAL(times_callback_called, 0);
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 0);

  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 1);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 2);
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));

  // make sure that stop actually stops it
  delay_us(2 * interval_us);

  TEST_ASSERT_EQUAL(times_callback_called, 2);
}

// Same, but for pca9539r initialization.
void test_bts_7200_current_sense_timer_pca9539r_works(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  Pca9539rGpioAddress test_select_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  Pca9539rGpioAddress test_enable_0_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_1 };
  Pca9539rGpioAddress test_enable_1_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts_7200_init_pca9539r(&s_storage, &settings));

  // make sure we don't start anything in init
  TEST_ASSERT_EQUAL(times_callback_called, 0);
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 0);

  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 1);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 2);

  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));

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
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 1);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 2);

  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));

  // make sure it's stopped
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 2);

  // start again
  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(times_callback_called, 3);

  // wait in a busy loop for the callback to be called
  while (times_callback_called == 3) {
  }

  TEST_ASSERT_EQUAL(times_callback_called, 4);

  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));

  // make sure it's stopped
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(times_callback_called, 4);
}

// Test init failure when the settings are invalid for stm32.
void test_bts_7200_current_sense_stm32_init_invalid_settings(void) {
  // start with invalid select pin
  GpioAddress select_pin = { .port = NUM_GPIO_PORTS, .pin = 0 };  // invalid
  GpioAddress sense_pin = { .port = 0, .pin = 0 };                // valid
  // EN0 and EN1 pins (both valid for now)
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &select_pin,
    .sense_pin = &sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_NOT_OK(bts_7200_init_stm32(&s_storage, &settings));

  // invalid sense pin
  select_pin.port = 0;
  sense_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(bts_7200_init_stm32(&s_storage, &settings));

  // otherwise valid
  sense_pin.port = 0;
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
}

// Same, but for pca9539r.
void test_bts_7200_current_sense_pca9539r_init_invalid_settings(void) {
  // start with invalid select pin
  Pca9539rGpioAddress select_pin = { .i2c_address = 0, .pin = NUM_PCA9539R_GPIO_PINS };  // invalid
  GpioAddress sense_pin = { .port = GPIO_PORT_A, .pin = 0 };                             // valid
  // EN0 and EN1 pins
  Pca9539rGpioAddress test_enable_0_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_1 };
  Pca9539rGpioAddress test_enable_1_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .select_pin = &select_pin,
    .sense_pin = &sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_NOT_OK(bts_7200_init_pca9539r(&s_storage, &settings));

  // invalid sense pin
  select_pin.pin = 0;
  sense_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(bts_7200_init_pca9539r(&s_storage, &settings));

  // otherwise valid
  sense_pin.port = GPIO_PORT_A;
  TEST_ASSERT_OK(bts_7200_init_pca9539r(&s_storage, &settings));
}

// Test that having a NULL callback works and we don't segfault.
void test_bts_7200_current_sense_null_callback(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = NULL,
  };

  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
  TEST_ASSERT_OK(bts_7200_start(&s_storage));
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));
}

// Test that bts_7200_get_measurement returns ok.
void test_bts_7200_current_sense_get_measurement_stm32_valid(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));

  uint16_t reading0 = 0, reading1 = 0;
  TEST_ASSERT_OK(bts_7200_get_measurement(&s_storage, &reading0, &reading1));
  LOG_DEBUG("STM32 readings: %d, %d\r\n", reading0, reading1);
}

// Same, but with pca9539r initialization.
void test_bts_7200_current_sense_get_measurement_pca9539r_valid(void) {
  Pca9539rGpioAddress test_select_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  Pca9539rGpioAddress test_enable_0_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_1 };
  Pca9539rGpioAddress test_enable_1_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts_7200_init_pca9539r(&s_storage, &settings));

  uint16_t reading0 = 0, reading1 = 0;
  TEST_ASSERT_OK(bts_7200_get_measurement(&s_storage, &reading0, &reading1));
  LOG_DEBUG("PCA9539R readings: %d, %d\r\n", reading0, reading1);
}

// Test that bts_7200_stop returns true only when it stops a timer
void test_bts_7200_current_sense_stop_return_behaviour(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
  TEST_ASSERT_EQUAL(false, bts_7200_stop(&s_storage));
  TEST_ASSERT_OK(bts_7200_start(&s_storage));
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));
  TEST_ASSERT_EQUAL(false, bts_7200_stop(&s_storage));
}

// Test that the context is actually passed to the function
void test_bts_7200_current_sense_context_passed(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;            // 0.5 ms
  void *context_pointer = &interval_us;  // arbitrary pointer
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .callback_context = context_pointer,
  };

  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
  TEST_ASSERT_OK(bts_7200_start(&s_storage));
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));
  TEST_ASSERT_EQUAL(received_context, context_pointer);
}

// Test enabling/disabling/getting value works with IN0 pin
void test_bts_7200_output_0_functions_work(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
  };
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));

  // Should initialize open
  TEST_ASSERT_EQUAL(bts_7200_get_output_0_enabled(&s_storage), false);

  // Close, then check
  TEST_ASSERT_OK(bts_7200_enable_output_0(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_0_enabled(&s_storage), true);

  // Open, then check
  TEST_ASSERT_OK(bts_7200_disable_output_0(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_0_enabled(&s_storage), false);

  // Make sure possible to open while already open
  TEST_ASSERT_OK(bts_7200_disable_output_0(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_0_enabled(&s_storage), false);

  // Make sure starting current sense doesn't cause issues
  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // Same tests as above:
  TEST_ASSERT_EQUAL(bts_7200_get_output_0_enabled(&s_storage), false);
  TEST_ASSERT_OK(bts_7200_enable_output_0(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_0_enabled(&s_storage), true);
  TEST_ASSERT_OK(bts_7200_disable_output_0(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_0_enabled(&s_storage), false);
  TEST_ASSERT_OK(bts_7200_disable_output_0(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_0_enabled(&s_storage), false);

  // Stop
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));
}

// Same as previous test, but with IN1 pin
void test_bts_7200_output_1_functions_work(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));

  // Should initialize open
  TEST_ASSERT_EQUAL(bts_7200_get_output_1_enabled(&s_storage), false);

  // Close, then check
  TEST_ASSERT_OK(bts_7200_enable_output_1(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_1_enabled(&s_storage), true);

  // Open, then check
  TEST_ASSERT_OK(bts_7200_disable_output_1(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_1_enabled(&s_storage), false);

  // Make sure possible to open while already open
  TEST_ASSERT_OK(bts_7200_disable_output_1(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_1_enabled(&s_storage), false);

  // Make sure starting current sense doesn't cause issues
  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // Same tests as above:
  TEST_ASSERT_EQUAL(bts_7200_get_output_1_enabled(&s_storage), false);
  TEST_ASSERT_OK(bts_7200_enable_output_1(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_1_enabled(&s_storage), true);
  TEST_ASSERT_OK(bts_7200_disable_output_1(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_1_enabled(&s_storage), false);
  TEST_ASSERT_OK(bts_7200_disable_output_1(&s_storage));
  TEST_ASSERT_EQUAL(bts_7200_get_output_1_enabled(&s_storage), false);

  // Stop
  TEST_ASSERT_EQUAL(true, bts_7200_stop(&s_storage));
}

// Test that fault callback called when voltage within fault range.
void test_bts_7200_faults_within_fault_range(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
    .fault_callback_context = context_pointer,  // pass in random variable
  };
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));

  s_times_fault_callback_called = 0;
  s_adc_measurement_0 = ADC_TEST_FAULT_VOLTAGE;
  s_adc_measurement_1 = ADC_TEST_FAULT_VOLTAGE;

  uint16_t meas0 = 0, meas1 = 0;
  bts_7200_get_measurement(&s_storage, &meas0, &meas1);

  TEST_ASSERT_EQUAL(s_times_fault_callback_called, 1);

  s_times_fault_callback_called = 0;
}

// Test that fault isn't called when voltage outside of fault range.
void test_bts_7200_no_fault_outside_of_fault_range(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
    .fault_callback_context = context_pointer,  // pass in random variable
  };
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));

  s_times_fault_callback_called = 0;
  s_adc_measurement_0 = ADC_EXPECTED_OK_VOLTAGE;
  s_adc_measurement_1 = ADC_EXPECTED_OK_VOLTAGE;

  uint16_t meas0 = 0, meas1 = 0;
  bts_7200_get_measurement(&s_storage, &meas0, &meas1);

  TEST_ASSERT_EQUAL(s_times_fault_callback_called, 0);

  s_times_fault_callback_called = 0;

  // Wait for soft_timers to expire before continuing
  // Why does having this cause a segfault????????????
  delay_ms(BTS7200_FAULT_RESTART_DELAY_MS + 10);
}

// Test that the fault callback gets called when running bts_7200_start, and
// there's no segfault/stack overflow.
// This test causes a segfault for some reason

void test_bts7200_fault_cb_called_from_start(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
  delay_us(2 * interval_us);
  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  s_times_fault_callback_called = 0;
  s_adc_measurement_0 = ADC_TEST_FAULT_VOLTAGE;
  s_adc_measurement_1 = ADC_TEST_FAULT_VOLTAGE;
  delay_us(2 * interval_us);  // wait for 2* interval
  TEST_ASSERT_TRUE(s_times_fault_callback_called > 0);
  TEST_ASSERT_TRUE(bts_7200_stop(&s_storage));
  s_times_fault_callback_called = 0;
}

// Test that fault cleared correctly, and that the correct IN pin(s) are toggled
// to clear the fault.
void test_bts_7200_handle_fault_clears_fault(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
  delay_us(2 * interval_us);
  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // enable outputs
  TEST_ASSERT_OK(bts_7200_enable_output_0(&s_storage));
  TEST_ASSERT_OK(bts_7200_enable_output_1(&s_storage));

  // Make sure not called early when ADC voltage normal
  delay_us(2 * interval_us);
  TEST_ASSERT_TRUE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_TRUE(bts_7200_get_output_1_enabled(&s_storage));

  // Fault
  s_adc_measurement_0 = ADC_TEST_FAULT_VOLTAGE;
  s_adc_measurement_1 = ADC_TEST_FAULT_VOLTAGE;
  delay_us(2 * interval_us);
  TEST_ASSERT_FALSE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_FALSE(bts_7200_get_output_1_enabled(&s_storage));

  // Set voltage back to normal so fault doesn't occur again
  s_adc_measurement_0 = ADC_EXPECTED_OK_VOLTAGE;
  s_adc_measurement_1 = ADC_EXPECTED_OK_VOLTAGE;

  // Make sure fault doesn't clear early
  delay_ms(80);
  TEST_ASSERT_FALSE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_FALSE(bts_7200_get_output_1_enabled(&s_storage));
  // Make sure fault clears after time elapsed and values return to normal
  delay_ms(40);
  TEST_ASSERT_TRUE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_TRUE(bts_7200_get_output_1_enabled(&s_storage));

  TEST_ASSERT_TRUE(bts_7200_stop(&s_storage));
}

// Test that fault context is passed on correctly on fault.
void test_bts_7200_fault_context_passed_on_fault(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
    .fault_callback_context = context_pointer,
  };
  uint16_t meas0 = 0, meas1 = 0;
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));

  s_adc_measurement_0 = ADC_TEST_FAULT_VOLTAGE;
  s_adc_measurement_1 = ADC_TEST_FAULT_VOLTAGE;

  // Measure to trigger fault, make sure context is passed okay
  TEST_ASSERT_OK(bts_7200_get_measurement(&s_storage, &meas0, &meas1));
  TEST_ASSERT_EQUAL(s_times_fault_callback_called, 1);
  TEST_ASSERT_EQUAL(fault_received_context, context_pointer);
}

// Test that trying to enable a pin during fault doesn't work.
void test_bts_7200_enable_fails_during_fault(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
  delay_us(2 * interval_us);
  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // Make sure outputs are off
  TEST_ASSERT_OK(bts_7200_disable_output_0(&s_storage));
  TEST_ASSERT_OK(bts_7200_disable_output_1(&s_storage));

  // Start fault, make sure fault handling works ok
  s_times_fault_callback_called = 0;
  s_adc_measurement_0 = ADC_TEST_FAULT_VOLTAGE;
  s_adc_measurement_1 = ADC_TEST_FAULT_VOLTAGE;
  delay_us(2 * interval_us);
  TEST_ASSERT_TRUE(s_times_fault_callback_called > 0);

  // Wait 40 ms; fault still in progress, shouldn't be possible to enable outputs.
  delay_ms(40);
  TEST_ASSERT_EQUAL(bts_7200_enable_output_0(&s_storage), STATUS_CODE_INTERNAL_ERROR);
  TEST_ASSERT_EQUAL(bts_7200_enable_output_1(&s_storage), STATUS_CODE_INTERNAL_ERROR);

  // Change ADC voltage back to OK so that fault doesn't get called after complete
  s_adc_measurement_0 = ADC_EXPECTED_OK_VOLTAGE;
  s_adc_measurement_1 = ADC_EXPECTED_OK_VOLTAGE;

  // Wait another 80 ms for fault handling to finish; should be able to enable outputs again.
  delay_ms(80);
  TEST_ASSERT_OK(bts_7200_enable_output_0(&s_storage));
  TEST_ASSERT_OK(bts_7200_enable_output_1(&s_storage));

  // Make sure outputs were actually enabled.
  TEST_ASSERT_TRUE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_TRUE(bts_7200_get_output_1_enabled(&s_storage));

  // Stop
  TEST_ASSERT_TRUE(bts_7200_stop(&s_storage));
}

// Test handling of a fault on only one input.
// Same as bts_7200_handle_fault_clears_fault, but with only IN1 fault.
void test_bts_7200_single_input_faults(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_select_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_0_pin = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress test_enable_1_pin = { .port = GPIO_PORT_A, .pin = 2 };
  uint32_t interval_us = 500;  // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7200Stm32Settings settings = {
    .select_pin = &test_select_pin,
    .sense_pin = &test_sense_pin,
    .enable_0_pin = &test_enable_0_pin,
    .enable_1_pin = &test_enable_1_pin,
    .interval_us = interval_us,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };
  TEST_ASSERT_OK(bts_7200_init_stm32(&s_storage, &settings));
  delay_us(2 * interval_us);
  TEST_ASSERT_OK(bts_7200_start(&s_storage));

  // enable outputs
  TEST_ASSERT_OK(bts_7200_enable_output_0(&s_storage));
  TEST_ASSERT_OK(bts_7200_enable_output_1(&s_storage));

  // Make sure not called early when ADC voltage normal
  delay_us(2 * interval_us);
  TEST_ASSERT_TRUE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_TRUE(bts_7200_get_output_1_enabled(&s_storage));

  // Fault, but only on IN1.
  s_adc_measurement_0 = ADC_EXPECTED_OK_VOLTAGE;
  s_adc_measurement_1 = ADC_TEST_FAULT_VOLTAGE;
  delay_us(2 * interval_us);
  TEST_ASSERT_TRUE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_FALSE(bts_7200_get_output_1_enabled(&s_storage));

  // Set voltage back to normal so fault doesn't get called again.
  s_adc_measurement_1 = ADC_EXPECTED_OK_VOLTAGE;

  // Make sure fault doesn't clear early
  delay_ms(80);
  TEST_ASSERT_TRUE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_FALSE(bts_7200_get_output_1_enabled(&s_storage));

  // Make sure fault clears after time elapsed and values return to normal
  delay_ms(40);
  TEST_ASSERT_TRUE(bts_7200_get_output_0_enabled(&s_storage));
  TEST_ASSERT_TRUE(bts_7200_get_output_1_enabled(&s_storage));

  TEST_ASSERT_TRUE(bts_7200_stop(&s_storage));
}
