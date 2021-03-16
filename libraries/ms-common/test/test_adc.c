#include "adc.h"

#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// temps in kelvin
#define ADC_INVALID_UNDER_TEMP 253
#define ADC_INVALID_OVER_TEMP 373

#define ADC_MOCK_READING 999

static volatile uint8_t s_callback_runs;
static volatile bool s_callback_ran;

static volatile uint8_t s_pin_callback_runs;
static volatile bool s_pin_callback_ran;

static const GpioAddress s_address[] = {
  { GPIO_PORT_A, 0 },
  { GPIO_PORT_A, 1 },
  { GPIO_PORT_A, 2 },
};

static const GpioAddress s_invalid_pin = { NUM_GPIO_PORTS, NUM_ADC_CHANNELS };
static const GpioAddress s_empty_pin = { GPIO_PORT_A, 3 };

void prv_callback(AdcChannel adc_channel, void *context) {
  s_callback_runs++;
  s_callback_ran = true;
}

void prv_callback_pin(GpioAddress address, void *context) {
  s_pin_callback_runs++;
  s_pin_callback_ran = true;
}

void prv_mock_read_callback(AdcChannel adc_channel, void *context) {
  uint16_t *reading = context;
  *reading = ADC_MOCK_READING;
}

// Check multiple samples to ensure they are within the correct range
void prv_adc_check_range(AdcChannel adc_channel) {
  uint16_t raw_reading = 0;
  uint16_t conv_reading = 0;

  for (uint8_t i = 0; i < 12; i++) {
    adc_read_raw(adc_channel, &raw_reading);
    TEST_ASSERT_TRUE(raw_reading <= 4095);

    adc_read_converted(adc_channel, &conv_reading);
    TEST_ASSERT_TRUE(conv_reading <= 3000);
  }
}

void prv_adc_check_range_pin(GpioAddress address) {
  uint16_t raw_reading = 0;
  uint16_t conv_reading = 0;

  for (uint8_t i = 0; i < 12; i++) {
    adc_read_raw_pin(address, &raw_reading);
    TEST_ASSERT_TRUE(raw_reading <= 4095);
    adc_read_converted_pin(address, &conv_reading);
    TEST_ASSERT_TRUE(conv_reading <= 3000);
  }
}

void setup_test() {
  GpioSettings settings = {
    GPIO_DIR_IN,        //
    GPIO_STATE_LOW,     //
    GPIO_RES_NONE,      //
    GPIO_ALTFN_ANALOG,  //
  };

  gpio_init();
  interrupt_init();
  soft_timer_init();  // for x86

  for (uint8_t i = ADC_CHANNEL_0; i < ADC_CHANNEL_2; i++) {
    gpio_init_pin(&s_address[i], &settings);
  }

  adc_init(ADC_MODE_SINGLE);
  s_callback_runs = 0;
  s_callback_ran = false;
  s_pin_callback_runs = 0;
  s_pin_callback_ran = false;
}

void teardown_test(void) {}

void test_set_channel(void) {
  // Check that channels can only be set with the correct channel arguments
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_set_channel(NUM_ADC_CHANNELS, true));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_0, true));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_1, true));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_2, true));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_0, false));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_1, false));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_2, false));
}

void test_set_callback(void) {
  // Check that callbacks can only be registered with the correct channel
  // arguments
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    adc_register_callback(NUM_ADC_CHANNELS, prv_callback, NULL));

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL));

  adc_set_channel(ADC_CHANNEL_0, true);
  adc_set_channel(ADC_CHANNEL_1, true);
  adc_set_channel(ADC_CHANNEL_2, true);

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL));
}

void test_single(void) {
  uint16_t reading;

  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel(ADC_CHANNEL_0, true);
  adc_set_channel(ADC_CHANNEL_1, true);
  adc_set_channel(ADC_CHANNEL_2, true);

  adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL);

  // Callbacks must not run in single mode unless a read occurs
  TEST_ASSERT_FALSE(s_callback_ran);
  TEST_ASSERT_EQUAL(false, s_callback_runs);

  // Ensure that the conversions happen once adc_read_value is called
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_raw(ADC_CHANNEL_0, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_raw(ADC_CHANNEL_1, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_raw(ADC_CHANNEL_2, &reading));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_read_raw(NUM_ADC_CHANNELS, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_read_raw(ADC_CHANNEL_3, &reading));

  while (!s_callback_ran) {
  }

  TEST_ASSERT_TRUE(s_callback_ran);
  TEST_ASSERT_TRUE(s_callback_runs > 0);
  TEST_ASSERT_TRUE(reading < 4095);
  TEST_ASSERT_FALSE(s_pin_callback_ran);
}

void test_continuous() {
  // Initialize ADC and check that adc_init() can properly reset the ADC
  adc_init(ADC_MODE_CONTINUOUS);

  adc_set_channel(ADC_CHANNEL_0, true);
  adc_set_channel(ADC_CHANNEL_1, true);
  adc_set_channel(ADC_CHANNEL_2, true);

  adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL);

  // Run a busy loop until a callback is triggered
  while (!s_callback_runs) {
  }

  TEST_ASSERT_TRUE(s_callback_ran);
  TEST_ASSERT_TRUE(s_callback_runs > 0);
}

void test_read_single(void) {
  // Check that both the raw readings and converted readings are within the
  // expected range
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel(ADC_CHANNEL_0, true);
  adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL);

  prv_adc_check_range(ADC_CHANNEL_0);
}

void test_read_continuous(void) {
  // Check that both the raw readings and converted readings are within the
  // expected range
  adc_init(ADC_MODE_CONTINUOUS);

  adc_set_channel(ADC_CHANNEL_0, true);
  adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL);

  prv_adc_check_range(ADC_CHANNEL_0);
}

void test_adc_get_channel() {
  AdcChannel adc_channel;
  GpioAddress address[] = {
    {
        .port = GPIO_PORT_A,
    },
    {
        .port = GPIO_PORT_B,
    },
    {
        .port = GPIO_PORT_C,
    },
  };

  address[0].pin = 0;
  TEST_ASSERT_OK(adc_get_channel(address[0], &adc_channel));
  address[0].pin = 8;
  TEST_ASSERT_NOT_OK(adc_get_channel(address[0], &adc_channel));

  address[1].pin = 0;
  TEST_ASSERT_OK(adc_get_channel(address[1], &adc_channel));
  address[1].pin = 2;
  TEST_ASSERT_NOT_OK(adc_get_channel(address[1], &adc_channel));

  address[2].pin = 0;
  TEST_ASSERT_OK(adc_get_channel(address[2], &adc_channel));
  address[2].pin = 6;
  TEST_ASSERT_NOT_OK(adc_get_channel(address[2], &adc_channel));
}

void test_adc_read_temp() {
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel(ADC_CHANNEL_TEMP, true);

  uint16_t reading;
  adc_read_converted(ADC_CHANNEL_TEMP, &reading);

  // ensure value is within reason
  TEST_ASSERT_TRUE(ADC_INVALID_UNDER_TEMP < reading);
  TEST_ASSERT_TRUE(ADC_INVALID_OVER_TEMP > reading);
}

void test_pin_set_channel(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_set_channel_pin(s_invalid_pin, true));

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel_pin(s_address[i], false));
  }

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel_pin(s_address[i], true));
  }
}

void test_pin_set_callback(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    adc_register_callback_pin(s_invalid_pin, prv_callback_pin, NULL));

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY,
                    adc_register_callback_pin(s_empty_pin, prv_callback_pin, NULL));

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                      adc_register_callback_pin(s_address[i], prv_callback_pin, NULL));
  }
}

void test_pin_single(void) {
  uint16_t reading;
  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_SINGLE);

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
    adc_set_channel_pin(s_address[i], true);
  }

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
    adc_register_callback_pin(s_address[i], prv_callback_pin, NULL);
  }

  // Callbacks must not run in single mode unless a read occurs
  TEST_ASSERT_FALSE(s_pin_callback_ran);
  TEST_ASSERT_EQUAL(false, s_pin_callback_runs);

  // Ensure that the conversions happen once adc_read_value is called
  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_raw_pin(s_address[i], &reading));
  }

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_read_raw_pin(s_invalid_pin, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_read_raw_pin(s_empty_pin, &reading));

  while (!s_pin_callback_ran) {
  }

  TEST_ASSERT_TRUE(s_pin_callback_ran);
  TEST_ASSERT_TRUE(s_pin_callback_runs > 0);
  TEST_ASSERT_TRUE(reading < 4095);

  TEST_ASSERT_FALSE(s_callback_ran);
}

void test_pin_continuous() {
  // Initialize ADC and check that adc_init() can properly reset the ADC
  adc_init(ADC_MODE_CONTINUOUS);

  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
    adc_set_channel_pin(s_address[i], true);
  }
  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_address); i++) {
    adc_register_callback_pin(s_address[i], prv_callback_pin, NULL);
  }

  // Run a busy loop until a callback is triggered
  while (!s_pin_callback_ran) {
  }

  TEST_ASSERT_TRUE(s_pin_callback_ran);
  TEST_ASSERT_TRUE(s_pin_callback_runs > 0);
  TEST_ASSERT_FALSE(s_callback_ran);
}

void test_pin_read_single(void) {
  // Check that both the raw readings and converted readings are within the
  // expected range
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(s_address[0], true);
  adc_register_callback_pin(s_address[0], prv_callback_pin, NULL);

  prv_adc_check_range_pin(s_address[0]);
}

void test_pin_read_continuous(void) {
  // Check that both the raw readings and converted readings are within the
  // expected range
  adc_init(ADC_MODE_CONTINUOUS);

  adc_set_channel_pin(s_address[0], true);
  adc_register_callback_pin(s_address[0], prv_callback_pin, NULL);

  prv_adc_check_range_pin(s_address[0]);
}

// test to help with other tests
void test_adc_mock_reading() {
  adc_init(ADC_MODE_SINGLE);
  GpioAddress address = { .port = GPIO_PORT_A, .pin = 0 };
  AdcChannel channel;
  adc_get_channel(address, &channel);
  adc_set_channel(channel, true);

  uint16_t reading;

  adc_register_callback(channel, prv_mock_read_callback, &reading);

  adc_read_raw(channel, &reading);
  TEST_ASSERT_TRUE(reading == ADC_MOCK_READING);
}
