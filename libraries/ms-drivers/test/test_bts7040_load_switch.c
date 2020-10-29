#include "adc.h"
#include "bts7040_load_switch.h"
#include "controller_board_pins.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_I2C_PORT I2C_PORT_2
#define TEST_I2C_ADDRESS 0x74

// 1.21k resistor
#define BTS7040_TEST_RESISTOR 1210

// 1 mA * BTS7040_TEST_RESISTOR
#define ADC_EXPECTED_OK_VOLTAGE 1210

// Values are capped at this on power_distribution boards
#define ADC_MIN_FAULT_VOLTAGE 3200
#define ADC_MAX_FAULT_VOLTAGE 3300

// Within fault range given
#define ADC_FAULT_VOLTAGE 3250

// Return ADC_EXPECTED_OK_VOLTAGE by default
#define ADC_DEFAULT_RETURNED_VOLTAGE_RAW ADC_EXPECTED_OK_VOLTAGE

// To mock ADC readings
static uint16_t s_adc_measurement = ADC_EXPECTED_OK_VOLTAGE;

static volatile uint16_t s_times_callback_called = 0;
static void *s_received_context;

static volatile uint16_t s_times_fault_callback_called = 0;
static void *s_fault_received_context;

static void prv_callback_increment(uint16_t reading, void *context) {
  s_times_callback_called++;
  s_received_context = context;
}

static void prv_fault_callback_increment(void *context) {
  s_times_fault_callback_called++;
  s_fault_received_context = context;
}

// Storage is global so we can call bts7040_stop to stop soft timers and avoid segfaults
static Bts7040Storage s_storage = { 0 };

// Mocks adc_read_converted to allow for changing the reading during testing.
// Note that this removes some of the interrupt functionality of the x86 adc implementation,
// but this shouldn't matter in this case.
StatusCode TEST_MOCK(adc_read_converted)(AdcChannel adc_channel, uint16_t *reading) {
  *reading = s_adc_measurement;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                //
    .sda = CONTROLLER_BOARD_ADDR_I2C2_SDA,  //
    .scl = CONTROLLER_BOARD_ADDR_I2C2_SCL,  //
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);

  s_times_callback_called = 0;
  s_times_fault_callback_called = 0;

  s_adc_measurement = ADC_EXPECTED_OK_VOLTAGE;

  s_received_context = NULL;
  s_fault_received_context = NULL;
}

void teardown_test(void) {
  // Stop so we don't segfault if an assert fails while the fault soft timer is in progress.
  bts7040_stop(&s_storage);
}

// Comprehensive happy-path test for STM32 initialization.
void test_bts7040_current_sense_timer_stm32_works(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };

  // EN pin
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };

  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));

  // make sure we don't start anything in init
  TEST_ASSERT_EQUAL(0, s_times_callback_called);
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(0, s_times_callback_called);

  TEST_ASSERT_OK(bts7040_start(&s_storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait in a busy loop for the callback to be called
  while (s_times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(2, s_times_callback_called);
  bts7040_stop(&s_storage);

  // make sure that stop actually stops it
  delay_us(2 * interval_us);

  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}

// Same, but for pca9539r initialization.
void test_bts7040_current_sense_timer_pca9539r_works(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN pin
  Pca9539rGpioAddress test_enable_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_1 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7040Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts7040_init_pca9539r(&s_storage, &settings));

  // make sure we don't start anything in init
  TEST_ASSERT_EQUAL(0, s_times_callback_called);
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(0, s_times_callback_called);

  TEST_ASSERT_OK(bts7040_start(&s_storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait in a busy loop for the callback to be called
  while (s_times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  bts7040_stop(&s_storage);

  // make sure that stop actually stops it
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}

// Test that we can init, start, stop, and start again and it works.
// Essentially the previous test done twice.
void test_bts7040_current_sense_restart(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));
  TEST_ASSERT_OK(bts7040_start(&s_storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait in a busy loop for the callback to be called
  while (s_times_callback_called == 1) {
  }

  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  bts7040_stop(&s_storage);

  // make sure it's stopped
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // start again
  TEST_ASSERT_OK(bts7040_start(&s_storage));

  // we call the callback and get good values before setting the timer
  TEST_ASSERT_EQUAL(3, s_times_callback_called);

  // wait in a busy loop for the callback to be called
  while (s_times_callback_called == 3) {
  }

  TEST_ASSERT_EQUAL(4, s_times_callback_called);

  bts7040_stop(&s_storage);

  // make sure it's stopped
  delay_us(2 * interval_us);
  TEST_ASSERT_EQUAL(4, s_times_callback_called);
}

// Test init failure when the settings are invalid for stm32.
void test_bts7040_current_sense_stm32_init_invalid_settings(void) {
  GpioAddress sense_pin = { .port = 0, .pin = 0 };  // valid
  // EN0 and EN1 pins (both valid for now)
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7040Stm32Settings settings = {
    .sense_pin = &sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
  };

  // invalid sense pin
  sense_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(bts7040_init_stm32(&s_storage, &settings));

  // otherwise valid
  sense_pin.port = 0;
  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));

  // invalid enable pin
  test_enable_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(bts7040_init_stm32(&s_storage, &settings));

  // change back to valid
  test_enable_pin.port = GPIO_PORT_A;
  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));
}

// Same, but for pca9539r.
void test_bts7040_current_sense_pca9539r_init_invalid_settings(void) {
  GpioAddress sense_pin = { .port = GPIO_PORT_A, .pin = 0 };  // valid
  // EN0 and EN1 pins
  Pca9539rGpioAddress test_enable_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_1 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7040Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .sense_pin = &sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
  };

  // invalid sense pin
  sense_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(bts7040_init_pca9539r(&s_storage, &settings));

  // otherwise valid
  sense_pin.port = GPIO_PORT_A;
  TEST_ASSERT_OK(bts7040_init_pca9539r(&s_storage, &settings));

  // invalid enable pin
  test_enable_pin.pin = NUM_PCA9539R_GPIO_PINS;
  TEST_ASSERT_NOT_OK(bts7040_init_pca9539r(&s_storage, &settings));

  // change back to valid
  test_enable_pin.pin = PCA9539R_PIN_IO0_0;
  TEST_ASSERT_OK(bts7040_init_pca9539r(&s_storage, &settings));
}

// Test that having a NULL callback works and we don't segfault.
void test_bts7040_current_sense_null_callback(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = NULL,
  };

  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));
  TEST_ASSERT_OK(bts7040_start(&s_storage));
  bts7040_stop(&s_storage);
}

// Test that bts7040_get_measurement returns ok.
void test_bts7040_current_sense_get_measurement_stm32_valid(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));

  uint16_t reading = 0;
  TEST_ASSERT_OK(bts7040_get_measurement(&s_storage, &reading));
  LOG_DEBUG("STM32 reading: %d\n", reading);
}

// Same, but with pca9539r initialization.
void test_bts7040_current_sense_get_measurement_pca9539r_valid(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  Pca9539rGpioAddress test_enable_pin = { .i2c_address = 0, .pin = PCA9539R_PIN_IO0_1 };  // EN pin
  uint32_t interval_us = 500;                                                             // 0.5 ms
  Bts7040Pca9539rSettings settings = {
    .i2c_port = TEST_I2C_PORT,
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
  };

  TEST_ASSERT_OK(bts7040_init_pca9539r(&s_storage, &settings));

  uint16_t reading = 0;
  TEST_ASSERT_OK(bts7040_get_measurement(&s_storage, &reading));
  LOG_DEBUG("PCA9539R reading: %d\n", reading);
}

// Test that the context is actually passed to the function
void test_bts7040_current_sense_context_passed(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN pin
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };
  uint32_t interval_us = 500;            // 0.5 ms
  void *context_pointer = &interval_us;  // arbitrary pointer
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .callback_context = context_pointer,
  };

  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));
  TEST_ASSERT_OK(bts7040_start(&s_storage));
  bts7040_stop(&s_storage);
  TEST_ASSERT_EQUAL(context_pointer, s_received_context);
}

// Test enabling/disabling/getting value works with IN pin
void test_bts7040_output_functions_work(void) {
  // these don't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  // EN0 and EN1 pins
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };
  uint32_t interval_us = 500;  // 0.5 ms
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
  };
  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));

  // Should initialize open
  TEST_ASSERT_FALSE(bts7040_get_output_enabled(&s_storage));

  // Close, then check
  TEST_ASSERT_OK(bts7040_enable_output(&s_storage));
  TEST_ASSERT_TRUE(bts7040_get_output_enabled(&s_storage));

  // Open, then check
  TEST_ASSERT_OK(bts7040_disable_output(&s_storage));
  TEST_ASSERT_FALSE(bts7040_get_output_enabled(&s_storage));

  // Make sure possible to open while already open
  TEST_ASSERT_OK(bts7040_disable_output(&s_storage));
  TEST_ASSERT_FALSE(bts7040_get_output_enabled(&s_storage));

  // Make sure starting current sense doesn't cause issues
  TEST_ASSERT_OK(bts7040_start(&s_storage));

  // Same tests as above:
  TEST_ASSERT_FALSE(bts7040_get_output_enabled(&s_storage));
  TEST_ASSERT_OK(bts7040_enable_output(&s_storage));
  TEST_ASSERT_TRUE(bts7040_get_output_enabled(&s_storage));
  TEST_ASSERT_OK(bts7040_disable_output(&s_storage));
  TEST_ASSERT_FALSE(bts7040_get_output_enabled(&s_storage));
  TEST_ASSERT_OK(bts7040_disable_output(&s_storage));
  TEST_ASSERT_FALSE(bts7040_get_output_enabled(&s_storage));

  // Stop
  bts7040_stop(&s_storage);
}

// Test that fault callback gets called when measurement is within fault range.
void test_bts7040_faults_within_fault_range(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
    .fault_callback_context = context_pointer,
  };
  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));

  s_times_fault_callback_called = 0;
  s_adc_measurement = ADC_FAULT_VOLTAGE;

  uint16_t meas = 0;
  bts7040_get_measurement(&s_storage, &meas);

  TEST_ASSERT_EQUAL(1, s_times_fault_callback_called);
}

// Test that fault isn't called when voltage outside of fault range.
void test_bts7040_no_fault_outside_of_fault_range(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
    .fault_callback_context = context_pointer,
  };
  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));

  s_times_fault_callback_called = 0;
  s_adc_measurement = ADC_EXPECTED_OK_VOLTAGE;

  uint16_t meas = 0;
  bts7040_get_measurement(&s_storage, &meas);

  TEST_ASSERT_EQUAL(0, s_times_fault_callback_called);
}

// Test that the fault callback gets called when running bts7040_start, and
// there's no segfault/stack overflow.
void test_bts7040_fault_cb_called_from_start(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };
  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));
  delay_us(2 * interval_us);
  TEST_ASSERT_OK(bts7040_start(&s_storage));

  s_times_fault_callback_called = 0;
  s_adc_measurement = ADC_FAULT_VOLTAGE;
  delay_us(2 * interval_us);  // wait for 2* interval
  TEST_ASSERT_TRUE(s_times_fault_callback_called > 0);
  bts7040_stop(&s_storage);
}

// Test that fault cleared correctly, and that the IN pin is toggled
// to clear the fault as required by the retry strategy.
void test_bts7040_handle_fault_clears_fault(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };
  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));
  delay_us(2 * interval_us);
  TEST_ASSERT_OK(bts7040_start(&s_storage));

  // Enable output
  TEST_ASSERT_OK(bts7040_enable_output(&s_storage));

  // Make sure not called early when ADC voltage normal
  delay_us(2 * interval_us);
  TEST_ASSERT_TRUE(bts7040_get_output_enabled(&s_storage));

  // Fault
  s_adc_measurement = ADC_FAULT_VOLTAGE;
  delay_us(2 * interval_us);
  TEST_ASSERT_FALSE(bts7040_get_output_enabled(&s_storage));

  // Set voltage back to normal so fault doesn't occur again
  s_adc_measurement = ADC_EXPECTED_OK_VOLTAGE;

  // Make sure fault doesn't clear early
  delay_ms(80);
  TEST_ASSERT_FALSE(bts7040_get_output_enabled(&s_storage));
  // Make sure fault clears after time elapsed and values return to normal
  delay_ms(40);
  TEST_ASSERT_TRUE(bts7040_get_output_enabled(&s_storage));

  bts7040_stop(&s_storage);
}

// Test that fault context is passed on correctly on fault.
void test_bts7040_fault_context_passed_on_fault(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
    .fault_callback_context = context_pointer,
  };

  uint16_t meas = 0;
  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));

  s_adc_measurement = ADC_FAULT_VOLTAGE;

  // Measure to trigger fault, make sure context is passed okay.
  // Measurement should return STATUS_CODE_INTERNAL_ERROR since a fault is detected
  TEST_ASSERT_NOT_OK(bts7040_get_measurement(&s_storage, &meas));
  TEST_ASSERT_EQUAL(1, s_times_fault_callback_called);
  TEST_ASSERT_EQUAL(context_pointer, s_fault_received_context);
}

// Test that trying to enable a pin during fault doesn't work.
void test_bts7040_enable_fails_during_fault(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };

  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));
  delay_us(2 * interval_us);
  TEST_ASSERT_OK(bts7040_start(&s_storage));

  // Make sure outputs are off
  TEST_ASSERT_OK(bts7040_disable_output(&s_storage));

  // Start fault, make sure fault handling works ok
  s_times_fault_callback_called = 0;
  s_adc_measurement = ADC_FAULT_VOLTAGE;
  delay_us(2 * interval_us);
  TEST_ASSERT_TRUE(s_times_fault_callback_called > 0);

  // Wait 40 ms; fault still in progress, shouldn't be possible to enable outputs.
  delay_ms(40);
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, bts7040_enable_output(&s_storage));

  // Change ADC voltage back to OK so that fault doesn't get called after complete
  s_adc_measurement = ADC_EXPECTED_OK_VOLTAGE;

  // Wait another 80 ms for fault handling to finish; should be able to enable output again.
  delay_ms(80);
  TEST_ASSERT_OK(bts7040_enable_output(&s_storage));

  // Make sure output was actually enabled.
  TEST_ASSERT_TRUE(bts7040_get_output_enabled(&s_storage));

  // Stop
  bts7040_stop(&s_storage);
}

// Make sure that bts7040_stop stops all soft timers and sets
// fault_in_progress to false for both pins.
void test_bts7040_stop_works(void) {
  // this doesn't matter (adc isn't reading anything) but can't be null
  GpioAddress test_sense_pin = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress test_enable_pin = { .port = GPIO_PORT_A, .pin = 1 };  // EN pin
  uint32_t interval_us = 500;                                       // 0.5 ms
  void *context_pointer = &interval_us;
  Bts7040Stm32Settings settings = {
    .sense_pin = &test_sense_pin,
    .enable_pin = &test_enable_pin,
    .interval_us = interval_us,
    .resistor = BTS7040_TEST_RESISTOR,
    .min_fault_voltage_mv = ADC_MIN_FAULT_VOLTAGE,
    .max_fault_voltage_mv = ADC_MAX_FAULT_VOLTAGE,
    .callback = &prv_callback_increment,
    .fault_callback = &prv_fault_callback_increment,
  };

  TEST_ASSERT_OK(bts7040_init_stm32(&s_storage, &settings));
  TEST_ASSERT_OK(bts7040_start(&s_storage));
  TEST_ASSERT_EQUAL(0, s_times_fault_callback_called);
  // Cause fault
  s_adc_measurement = ADC_FAULT_VOLTAGE;

  // Make sure fault happens
  delay_us(interval_us * 2);
  TEST_ASSERT_TRUE(s_times_fault_callback_called > 0);

  // Soft timers for fault should be in progress
  TEST_ASSERT_TRUE(soft_timer_remaining_time(s_storage.enable_pin.fault_timer_id) > 0);

  // Stop
  bts7040_stop(&s_storage);

  // Verify
  TEST_ASSERT_EQUAL(SOFT_TIMER_INVALID_TIMER, s_storage.measurement_timer_id);
  TEST_ASSERT_EQUAL(SOFT_TIMER_INVALID_TIMER, s_storage.enable_pin.fault_timer_id);
  TEST_ASSERT_FALSE(s_storage.enable_pin.fault_in_progress);
}
