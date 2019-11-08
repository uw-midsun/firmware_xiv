#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

static volatile uint8_t s_callback_runs = 0;
static volatile bool s_callback_ran = false;

void prv_callback(AdcChannel adc_channel, void *context) {
  s_callback_runs++;
  s_callback_ran = true;
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

void setup_test() {
  GpioSettings settings = {
    GPIO_DIR_IN,        //
    GPIO_STATE_LOW,     //
    GPIO_RES_NONE,      //
    GPIO_ALTFN_ANALOG,  //
  };

  GpioAddress address[] = { {
                                GPIO_PORT_A,  //
                                0,            //
                            },
                            {
                                GPIO_PORT_A,  //
                                1,            //
                            },
                            {
                                GPIO_PORT_A,  //
                                2,            //
                            } };

  gpio_init();
  interrupt_init();

  for (uint8_t i = ADC_CHANNEL_0; i < ADC_CHANNEL_2; i++) {
    gpio_init_pin(&address[i], &settings);
  }

  adc_init(ADC_MODE_SINGLE);
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
}

void test_continuous() {
  s_callback_runs = 0;
  s_callback_ran = false;

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
