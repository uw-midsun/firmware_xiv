#include "sense_temperature.h"

#include <stdbool.h>

#include "adc.h"
#include "data_store.h"
#include "gpio.h"
#include "log.h"
#include "sense.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_ADC_RAW_READING 0x800

static SenseCallback s_sense_callbacks[MAX_THERMISTORS];
static void *s_sense_callback_contexts[MAX_THERMISTORS];
static uint8_t s_num_sense_callbacks;

StatusCode TEST_MOCK(sense_register)(SenseCallback callback, void *context) {
  s_sense_callbacks[s_num_sense_callbacks] = callback;
  s_sense_callback_contexts[s_num_sense_callbacks] = context;
  TEST_ASSERT_NOT_NULL(s_sense_callbacks[s_num_sense_callbacks]);
  s_num_sense_callbacks++;
  return STATUS_CODE_OK;
}

static AdcChannel s_adc_channel_passed;
static StatusCode s_adc_read_raw_ret;

StatusCode TEST_MOCK(adc_read_raw)(AdcChannel adc_channel, uint16_t *reading) {
  s_adc_channel_passed = adc_channel;
  *reading = TEST_ADC_RAW_READING;
  return s_adc_read_raw_ret;
}

static void prv_trigger_sense_cycle(void) {
  for (uint8_t i = 0; i < s_num_sense_callbacks; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
  }
}

void setup_test(void) {
  gpio_init();
  adc_init(ADC_MODE_SINGLE);
  data_store_init();
  s_num_sense_callbacks = 0;
  s_adc_channel_passed = NUM_ADC_CHANNELS;
  s_adc_read_raw_ret = STATUS_CODE_OK;
}
void teardown_test(void) {}

// Test that everything works correctly for a sense cycle with a single thermistor.
void test_single_thermistor_cycle(void) {
  bool is_set;
  uint32_t set_value;
  SenseTemperatureSettings settings = {
    .thermistor_settings = { { RTD_THERMISTOR, { GPIO_PORT_A, 0 } } },
    .num_thermistors = 1,
  };
  TEST_ASSERT_OK(sense_temperature_init(&settings));

  // we use the ADC channel later, so get it now
  AdcChannel adc_channel = NUM_ADC_CHANNELS;
  adc_get_channel(settings.thermistor_settings[0].thermistor_address, &adc_channel);

  // nothing was set yet
  data_store_get_is_set(DATA_POINT_TEMPERATURE(0), &is_set);
  TEST_ASSERT_FALSE(is_set);

  // it should be set after the sense cycle (and we should have read from the right channel)
  prv_trigger_sense_cycle();
  TEST_ASSERT_EQUAL(adc_channel, s_adc_channel_passed);
  data_store_get_is_set(DATA_POINT_TEMPERATURE(0), &is_set);
  TEST_ASSERT_TRUE(is_set);
  data_store_get(DATA_POINT_TEMPERATURE(0), &set_value);
  TEST_ASSERT_EQUAL(TEST_ADC_RAW_READING, set_value);
}

// Test that everything works correctly for a single cycle with the max number of thermistors.
void test_max_thermistors_cycle(void) {
  bool is_set;
  uint32_t set_value;
  SenseTemperatureSettings settings = {
    .num_thermistors = MAX_THERMISTORS,
  };
  // take the pins sequentially from PA0 on
  for (uint8_t thermistor = 0; thermistor < MAX_THERMISTORS; thermistor++) {
    settings.thermistor_settings[thermistor].thermistor_address.port =
        thermistor / GPIO_PINS_PER_PORT;
    settings.thermistor_settings[thermistor].thermistor_address.pin =
        thermistor % GPIO_PINS_PER_PORT;
  }
  TEST_ASSERT_OK(sense_temperature_init(&settings));

  // nothing was set yet
  for (uint8_t thermistor = 0; thermistor < MAX_THERMISTORS; thermistor++) {
    data_store_get_is_set(DATA_POINT_TEMPERATURE(thermistor), &is_set);
    TEST_ASSERT_FALSE(is_set);
  }

  // everything should be set correctly after the sense cycle
  prv_trigger_sense_cycle();
  for (uint8_t thermistor = 0; thermistor < MAX_THERMISTORS; thermistor++) {
    data_store_get_is_set(DATA_POINT_TEMPERATURE(thermistor), &is_set);
    TEST_ASSERT_TRUE(is_set);
    data_store_get(DATA_POINT_TEMPERATURE(thermistor), &set_value);
    TEST_ASSERT_EQUAL(TEST_ADC_RAW_READING, set_value);
  }
}

// Test that we can recover after a temporary fault (single thermistor).
void test_adc_read_fail(void) {
  bool is_set;
  uint32_t set_value;
  SenseTemperatureSettings settings = {
    .thermistor_settings = { { RTD_THERMISTOR, { GPIO_PORT_A, 0 } } },
    .num_thermistors = 1,
  };
  TEST_ASSERT_OK(sense_temperature_init(&settings));
  AdcChannel adc_channel = NUM_ADC_CHANNELS;
  adc_get_channel(settings.thermistor_settings[0].thermistor_address, &adc_channel);

  // fault: should show warning but not set value
  s_adc_read_raw_ret = STATUS_CODE_INTERNAL_ERROR;
  LOG_DEBUG("Testing ADC fault return value: there should be exactly one warning...\n");
  prv_trigger_sense_cycle();
  data_store_get_is_set(DATA_POINT_TEMPERATURE(0), &is_set);
  TEST_ASSERT_FALSE(is_set);

  // we should be able to recover
  s_adc_read_raw_ret = STATUS_CODE_OK;
  prv_trigger_sense_cycle();
  TEST_ASSERT_EQUAL(adc_channel, s_adc_channel_passed);
  data_store_get_is_set(DATA_POINT_TEMPERATURE(0), &is_set);
  TEST_ASSERT_TRUE(is_set);
  data_store_get(DATA_POINT_TEMPERATURE(0), &set_value);
  TEST_ASSERT_EQUAL(TEST_ADC_RAW_READING, set_value);
}

// Test that we fail gracefully when passed invalid settings.
void test_invalid_settings(void) {
  // too many thermistors but valid GPIO pins
  SenseTemperatureSettings invalid_settings = {
    .num_thermistors = MAX_THERMISTORS + 1,
  };
  for (uint8_t thermistor = 0; thermistor < MAX_THERMISTORS; thermistor++) {
    invalid_settings.thermistor_settings[thermistor].thermistor_address.port =
        thermistor / GPIO_PINS_PER_PORT;
    invalid_settings.thermistor_settings[thermistor].thermistor_address.pin =
        thermistor % GPIO_PINS_PER_PORT;
  }
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_temperature_init(&invalid_settings));

  // invalid GPIO pin
  invalid_settings.num_thermistors = MAX_THERMISTORS;
  invalid_settings.thermistor_settings[0].thermistor_address.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(sense_temperature_init(&invalid_settings));

  // null settings
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_temperature_init(NULL));
}
