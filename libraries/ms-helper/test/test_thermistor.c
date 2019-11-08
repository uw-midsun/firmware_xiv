#include <inttypes.h>
#include "adc.h"
#include "gpio.h"
#include "log.h"
#include "test_helpers.h"
#include "thermistor.h"
#include "unity.h"

#define THERMISTOR_TEMPERATURE_TOLERANCE_DECICELSIUS 5

// The tests cases return different voltage values depending on the inputted adc
// channel inputs Each value is used for a different test case Channel 0~3 used
// for thermistor = R1. Channel 4~6 used for thermistor = R2. Channel 7 & REF
// used for producing errors.
StatusCode TEST_MOCK(adc_read_converted)(AdcChannel adc_channel, uint16_t *reading) {
  switch (adc_channel) {
    case (ADC_CHANNEL_0):
      *reading = 2900;  // Used for testing out of range temperatures
      break;
    case (ADC_CHANNEL_1):
      *reading = 1926;  // Used for testing a normal temperature calculation
      break;
    case (ADC_CHANNEL_2):
      *reading = 2193;  // Tests lower bound of the temperature lookup table
      break;
    case (ADC_CHANNEL_3):
      *reading = 267;  // Tests upper bound of the temperature lookup table
      break;
    case (ADC_CHANNEL_4):
      *reading = 1074;  // Used for testing a normal temperature calculation
      break;
    case (ADC_CHANNEL_5):
      *reading = 807;  // Tests lower bound of the temperature lookup table
      break;
    case (ADC_CHANNEL_6):
      *reading = 2733;  // Tests upper bound of the temperature lookup table
      break;
    case (ADC_CHANNEL_7):  // Used to test out of bound lookup table values
      *reading = 0;
      break;
    case (ADC_CHANNEL_REF):  // The VDDA voltage used for these test scenarios
      *reading = 3000;
      break;
    default:

      break;
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(adc_get_channel)(GpioAddress address, AdcChannel *adc_channel) {
  *adc_channel = address.pin;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(adc_set_channel)(AdcChannel adc_channel, bool new_state) {
  return STATUS_CODE_OK;
}

void setup_test(void) {}

void teardown_test(void) {}

// Tests thermistor calculations with thermistor = R2
// Tests the thermistor at a standard temperature(10 degrees)
void test_thermistor_normal(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 1,
  };
  ThermistorStorage storage;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R2));

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
  TEST_ASSERT_UINT16_WITHIN(THERMISTOR_TEMPERATURE_TOLERANCE_DECICELSIUS, 100, reading);
}

// Tests the lower bound of the resistance-temperature lookup table (0 deg)
void test_thermistor_min_temp(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 2,
  };
  ThermistorStorage storage;

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R2));
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
  TEST_ASSERT_UINT16_WITHIN(THERMISTOR_TEMPERATURE_TOLERANCE_DECICELSIUS, 0, reading);
}

// Tests the upper bound of the resistance-temperature lookup table (100 deg)
void test_thermistor_max_temp(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 3,
  };
  ThermistorStorage storage;

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R2));
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
  TEST_ASSERT_UINT16_WITHIN(THERMISTOR_TEMPERATURE_TOLERANCE_DECICELSIUS, 1000, reading);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Tests thermistor calculations with thermistor = R1
// Tests the thermistor at a standard temperature(10 degrees)
void test_thermistor_normal_alt(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 4,
  };
  ThermistorStorage storage;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R1));

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  TEST_ASSERT_UINT16_WITHIN(THERMISTOR_TEMPERATURE_TOLERANCE_DECICELSIUS, 100, reading);
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
}

// Tests the lower bound of the resistance-temperature lookup table (0 deg)
void test_thermistor_min_temp_alt(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 5,
  };
  ThermistorStorage storage;

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R1));
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
  TEST_ASSERT_UINT16_WITHIN(THERMISTOR_TEMPERATURE_TOLERANCE_DECICELSIUS, 0, reading);
}

// Tests the upper bound of the resistance-temperature lookup table (100 deg)
void test_thermistor_max_temp_alt(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };
  ThermistorStorage storage;

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R1));
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
  TEST_ASSERT_UINT16_WITHIN(THERMISTOR_TEMPERATURE_TOLERANCE_DECICELSIUS, 1000, reading);
}

// Tests for a temperature greater the lookup tables ranges
void test_thermistor_exceed_range(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };
  ThermistorStorage storage;

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R2));
  TEST_ASSERT_NOT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
}

// Tests for a temperature smaller than the lookup tables ranges
void test_thermistor_under_range(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 7,
  };
  ThermistorStorage storage;

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R2));
  // Adds one millivolt as to not trigger the NULL read case
  reading++;
  TEST_ASSERT_NOT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
}

// Tests for NULL node voltage reading
void test_zero_node_voltage(void) {
  GpioAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 7,
  };
  ThermistorStorage storage;

  uint16_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, THERMISTOR_POSITION_R2));
  LOG_DEBUG("Temperature: %" PRIuLEAST16 "\n", reading);
  TEST_ASSERT_NOT_OK(thermistor_get_temp(&storage, &reading));
}

// Testing the temperature search function
void test_temperature_calculation(void) {
  uint16_t temperature = 0;
  // 10 Degrees
  thermistor_calculate_temp(17925, &temperature);
  TEST_ASSERT_UINT16_WITHIN(200, 100, temperature);

  // 25 Degrees
  thermistor_calculate_temp(10000, &temperature);
  TEST_ASSERT_UINT16_WITHIN(200, 250, temperature);

  // 50 Degrees
  thermistor_calculate_temp(4160, &temperature);
  TEST_ASSERT_UINT16_WITHIN(200, 500, temperature);

  // 75 Degrees
  thermistor_calculate_temp(1924, &temperature);
  TEST_ASSERT_UINT16_WITHIN(200, 750, temperature);

  // 90 Degrees
  thermistor_calculate_temp(1268, &temperature);
  TEST_ASSERT_UINT16_WITHIN(200, 900, temperature);
}

// Test temperature to resistance
void test_resistance_lookup(void) {
  uint16_t resistance = 0;
  // Over 100 degrees
  TEST_ASSERT_NOT_OK(thermistor_calculate_resistance(1010, &resistance));

  // Exact: No interpolation required
  // 10 Degrees
  thermistor_calculate_resistance(100, &resistance);
  TEST_ASSERT_UINT16_WITHIN(1, 17925, resistance);

  // 25 Degrees
  thermistor_calculate_resistance(250, &resistance);
  TEST_ASSERT_UINT16_WITHIN(1, 10000, resistance);

  // 65 Degrees
  thermistor_calculate_resistance(650, &resistance);
  TEST_ASSERT_UINT16_WITHIN(1, 2586, resistance);

  // At exactly 100 degrees
  thermistor_calculate_resistance(1000, &resistance);
  TEST_ASSERT_UINT16_WITHIN(1, 973, resistance);

  // Test some interpolated values
  // 12.5 degrees
  thermistor_calculate_resistance(125, &resistance);
  TEST_ASSERT_UINT16_WITHIN(1, 16210, resistance);

  // 50.5 degrees
  thermistor_calculate_resistance(505, &resistance);
  TEST_ASSERT_UINT16_WITHIN(1, 4093, resistance);

  // 74.7 degrees
  thermistor_calculate_resistance(747, &resistance);
  TEST_ASSERT_UINT16_WITHIN(1, 1941, resistance);

  // 75.5 degrees
  thermistor_calculate_resistance(755, &resistance);
  TEST_ASSERT_UINT16_WITHIN(1, 1897, resistance);
}
