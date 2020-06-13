#include "delay.h"
#include "event_queue.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "mcp3427_adc.h"
#include "mcp3427_adc_defs.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_ADDR_PIN_0 MCP3427_PIN_STATE_LOW
#define TEST_ADDR_PIN_1 MCP3427_PIN_STATE_LOW
#define TEST_I2C_PORT I2C_PORT_2

// these are used for a second address in |test_multiple_mcp3427s|
#define TEST_SECONDARY_ADDR_PIN_0 MCP3427_PIN_STATE_HIGH
#define TEST_SECONDARY_ADDR_PIN_1 MCP3427_PIN_STATE_HIGH
#define TEST_SECONDARY_I2C_PORT I2C_PORT_2

#define TEST_AMP_GAIN MCP3427_AMP_GAIN_1

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

typedef enum {
  TEST_MCP3427_DATA_TRIGGER_EVENT = 0,
  TEST_MCP3427_DATA_READY_EVENT,
  NUM_MCP3427_TEST_EVENTS,
} TestMcp3427Event;

// Like MS_TEST_HELPERS_ASSERT_EVENT, but only checks ID and not data
#define TEST_ASSERT_EVENT_WITH_ID(e, event_id) \
  ({                                           \
    TEST_ASSERT_OK(event_process(&(e)));       \
    TEST_ASSERT_EQUAL((event_id), (e).id);     \
  })

static uint16_t s_times_callback_called = 0;
static void *s_callback_context = NULL;
static uint16_t s_times_fault_callback_called = 0;
static void *s_fault_callback_context = NULL;

static void prv_callback(int16_t value_ch1, int16_t value_ch2, void *context) {
  LOG_DEBUG("Callback called: value_ch1=%d, value_ch2=%d\n", value_ch1, value_ch2);
  s_times_callback_called++;
  s_callback_context = context;
}

static void prv_fault_callback(void *context) {
  LOG_WARN("Fault callback called!");
  s_times_fault_callback_called++;
  s_fault_callback_context = context;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = TEST_CONFIG_PIN_I2C_SDA,
    .scl = TEST_CONFIG_PIN_I2C_SCL,
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);

  s_times_callback_called = s_times_fault_callback_called = 0;
  s_callback_context = s_fault_callback_context = NULL;
}
void teardown_test(void) {}

// Test a single data fetching round (2 conversions) in the current MCP3427 settings.
static void prv_test_data_round(Mcp3427Storage *storage, Event *e, void *callback_context) {
  // triggering CH1 conversion, a data ready event should be raised in MCP3427_MAX_CONV_TIME_MS
  TEST_ASSERT_OK(mcp3427_process_event(e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_ms(MCP3427_MAX_CONV_TIME_MS);
  TEST_ASSERT_EVENT_WITH_ID(*e, TEST_MCP3427_DATA_READY_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(0, s_times_callback_called);  // callback not called yet

  // reading CH1 data, a data trigger event should be raised immediately
  TEST_ASSERT_OK(mcp3427_process_event(e));
  TEST_ASSERT_EVENT_WITH_ID(*e, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(0, s_times_callback_called);  // callback not called yet, only CH1 read

  // triggering CH2 conversion, data ready event raised in MCP3427_MAX_CONV_TIME_MS
  TEST_ASSERT_OK(mcp3427_process_event(e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_ms(MCP3427_MAX_CONV_TIME_MS);
  TEST_ASSERT_EVENT_WITH_ID(*e, TEST_MCP3427_DATA_READY_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(0, s_times_callback_called);  // callback not called yet

  // reading CH2 data, callback called and data trigger event raised
  TEST_ASSERT_OK(mcp3427_process_event(e));
  TEST_ASSERT_EVENT_WITH_ID(*e, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(1, s_times_callback_called);            // callback called since it's CH2
  TEST_ASSERT_EQUAL(callback_context, s_callback_context);  // context was passed

  // the fault callback shouldn't have been called ever
  TEST_ASSERT_EQUAL(0, s_times_fault_callback_called);
  TEST_ASSERT_EQUAL(NULL, s_fault_callback_context);

  // clean up for a possible next round
  s_callback_context = NULL;
  s_times_callback_called = 0;
}

// Test initialization and that the callback is called when it's supposed to be without faults.
// 12 bit/continuous is the fastest speed, so failure on STM32 means something's very wrong.
// This also serves as a comprehensive happy-path test.
void test_12_bit_continuous_sampling_and_multiple_rounds(void) {
  Event e = { 0 };
  Mcp3427Settings settings = {
    .sample_rate = MCP3427_SAMPLE_RATE_12_BIT,
    .conversion_mode = MCP3427_CONVERSION_MODE_CONTINUOUS,
    .addr_pin_0 = TEST_ADDR_PIN_0,
    .addr_pin_1 = TEST_ADDR_PIN_1,
    .amplifier_gain = TEST_AMP_GAIN,
    .port = TEST_I2C_PORT,
    .adc_data_ready_event = TEST_MCP3427_DATA_READY_EVENT,
    .adc_data_trigger_event = TEST_MCP3427_DATA_TRIGGER_EVENT,
  };
  Mcp3427Storage storage;
  TEST_ASSERT_OK(mcp3427_init(&storage, &settings));

  // arbitrary variables to serve as contexts
  uint8_t callback_context, fault_context;

  TEST_ASSERT_OK(mcp3427_register_callback(&storage, prv_callback, &callback_context));
  TEST_ASSERT_OK(mcp3427_register_fault_callback(&storage, prv_fault_callback, &fault_context));

  // start it and process the initial event
  TEST_ASSERT_OK(mcp3427_start(&storage));
  TEST_ASSERT_EVENT_WITH_ID(e, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // test two rounds to make sure the loop succeeds
  prv_test_data_round(&storage, &e, &callback_context);
  LOG_DEBUG("Data round 1 succeeded\n");
  prv_test_data_round(&storage, &e, &callback_context);
  LOG_DEBUG("Data round 2 succeeded\n");
}

// Test a data conversion round with 16-bit continuous sampling.
// This is the slowest continuous speed, so succeeding on STM32 means all continuous speeds work.
void test_16_bit_continuous_sampling(void) {
  Event e = { 0 };
  Mcp3427Settings settings = {
    .sample_rate = MCP3427_SAMPLE_RATE_16_BIT,
    .conversion_mode = MCP3427_CONVERSION_MODE_CONTINUOUS,
    .addr_pin_0 = TEST_ADDR_PIN_0,
    .addr_pin_1 = TEST_ADDR_PIN_1,
    .amplifier_gain = TEST_AMP_GAIN,
    .port = TEST_I2C_PORT,
    .adc_data_ready_event = TEST_MCP3427_DATA_READY_EVENT,
    .adc_data_trigger_event = TEST_MCP3427_DATA_TRIGGER_EVENT,
  };
  Mcp3427Storage storage;
  TEST_ASSERT_OK(mcp3427_init(&storage, &settings));

  TEST_ASSERT_OK(mcp3427_register_callback(&storage, prv_callback, NULL));
  TEST_ASSERT_OK(mcp3427_register_fault_callback(&storage, prv_fault_callback, NULL));

  // start it and process the initial event
  TEST_ASSERT_OK(mcp3427_start(&storage));
  TEST_ASSERT_EVENT_WITH_ID(e, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  prv_test_data_round(&storage, &e, NULL);
}

// Test a data conversion round with 12-bit one-shot sampling.
// This is the fastest one-shot speed, so failure on STM32 means no one-shot speed will work.
void test_12_bit_one_shot_sampling(void) {
  Event e = { 0 };
  Mcp3427Settings settings = {
    .sample_rate = MCP3427_SAMPLE_RATE_12_BIT,
    .conversion_mode = MCP3427_CONVERSION_MODE_ONE_SHOT,
    .addr_pin_0 = TEST_ADDR_PIN_0,
    .addr_pin_1 = TEST_ADDR_PIN_1,
    .amplifier_gain = TEST_AMP_GAIN,
    .port = TEST_I2C_PORT,
    .adc_data_ready_event = TEST_MCP3427_DATA_READY_EVENT,
    .adc_data_trigger_event = TEST_MCP3427_DATA_TRIGGER_EVENT,
  };
  Mcp3427Storage storage;
  TEST_ASSERT_OK(mcp3427_init(&storage, &settings));

  TEST_ASSERT_OK(mcp3427_register_callback(&storage, prv_callback, NULL));
  TEST_ASSERT_OK(mcp3427_register_fault_callback(&storage, prv_fault_callback, NULL));

  // start it and process the initial event
  TEST_ASSERT_OK(mcp3427_start(&storage));
  TEST_ASSERT_EVENT_WITH_ID(e, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  prv_test_data_round(&storage, &e, NULL);
}

// Test a data conversion round with 16-bit one-shot sampling.
// This is the slowest one-shot speed, so succeeding on STM32 means all one-shot speeds work.
void test_16_bit_one_shot_sampling(void) {
  Event e = { 0 };
  Mcp3427Settings settings = {
    .sample_rate = MCP3427_SAMPLE_RATE_16_BIT,
    .conversion_mode = MCP3427_CONVERSION_MODE_ONE_SHOT,
    .addr_pin_0 = TEST_ADDR_PIN_0,
    .addr_pin_1 = TEST_ADDR_PIN_1,
    .amplifier_gain = TEST_AMP_GAIN,
    .port = TEST_I2C_PORT,
    .adc_data_ready_event = TEST_MCP3427_DATA_READY_EVENT,
    .adc_data_trigger_event = TEST_MCP3427_DATA_TRIGGER_EVENT,
  };
  Mcp3427Storage storage;
  TEST_ASSERT_OK(mcp3427_init(&storage, &settings));

  TEST_ASSERT_OK(mcp3427_register_callback(&storage, prv_callback, NULL));
  TEST_ASSERT_OK(mcp3427_register_fault_callback(&storage, prv_fault_callback, NULL));

  // start it and process the initial event
  TEST_ASSERT_OK(mcp3427_start(&storage));
  TEST_ASSERT_EVENT_WITH_ID(e, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  prv_test_data_round(&storage, &e, NULL);
}

// Test that we can have multiple chips simultaneously and neither interfere with each other.
void test_multiple_mcp3427s(void) {
  Event e1 = { 0 }, e2 = { 0 };
  Mcp3427Settings settings = {
    .sample_rate = MCP3427_SAMPLE_RATE_16_BIT,
    .conversion_mode = MCP3427_CONVERSION_MODE_ONE_SHOT,
    .addr_pin_0 = TEST_ADDR_PIN_0,
    .addr_pin_1 = TEST_ADDR_PIN_1,
    .amplifier_gain = TEST_AMP_GAIN,
    .port = TEST_I2C_PORT,
    .adc_data_ready_event = TEST_MCP3427_DATA_READY_EVENT,
    .adc_data_trigger_event = TEST_MCP3427_DATA_TRIGGER_EVENT,
  };
  Mcp3427Storage storage1, storage2;
  TEST_ASSERT_OK(mcp3427_init(&storage1, &settings));

  // modify for the second MCP3427
  settings.addr_pin_0 = TEST_SECONDARY_ADDR_PIN_0;
  settings.addr_pin_1 = TEST_SECONDARY_ADDR_PIN_1;
  settings.port = TEST_SECONDARY_I2C_PORT;
  TEST_ASSERT_OK(mcp3427_init(&storage2, &settings));

  TEST_ASSERT_OK(mcp3427_register_callback(&storage1, prv_callback, NULL));
  TEST_ASSERT_OK(mcp3427_register_callback(&storage2, prv_callback, NULL));
  TEST_ASSERT_OK(mcp3427_register_fault_callback(&storage1, prv_fault_callback, NULL));
  TEST_ASSERT_OK(mcp3427_register_fault_callback(&storage2, prv_fault_callback, NULL));

  TEST_ASSERT_OK(mcp3427_start(&storage1));
  TEST_ASSERT_OK(mcp3427_start(&storage2));

  // Process the start data trigger events
  // Passing both events to |mcp3427_process_event| should process both for the appropriate storages
  TEST_ASSERT_EVENT_WITH_ID(e1, TEST_MCP3427_DATA_TRIGGER_EVENT);
  TEST_ASSERT_EVENT_WITH_ID(e2, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // CH1 trigger: wait for data ready event
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(mcp3427_process_event(&e1));
  TEST_ASSERT_OK(mcp3427_process_event(&e2));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(0, s_times_callback_called);
  delay_ms(MCP3427_MAX_CONV_TIME_MS);
  TEST_ASSERT_EVENT_WITH_ID(e1, TEST_MCP3427_DATA_READY_EVENT);
  TEST_ASSERT_EVENT_WITH_ID(e2, TEST_MCP3427_DATA_READY_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(0, s_times_callback_called);

  // CH1 read: data trigger event raised immediately
  TEST_ASSERT_OK(mcp3427_process_event(&e1));
  TEST_ASSERT_OK(mcp3427_process_event(&e2));
  TEST_ASSERT_EVENT_WITH_ID(e1, TEST_MCP3427_DATA_TRIGGER_EVENT);
  TEST_ASSERT_EVENT_WITH_ID(e2, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(0, s_times_callback_called);  // callback not called yet, only CH1 read

  // CH2 trigger: wait for data ready event
  TEST_ASSERT_OK(mcp3427_process_event(&e1));
  TEST_ASSERT_OK(mcp3427_process_event(&e2));
  delay_ms(MCP3427_MAX_CONV_TIME_MS);
  TEST_ASSERT_EVENT_WITH_ID(e1, TEST_MCP3427_DATA_READY_EVENT);
  TEST_ASSERT_EVENT_WITH_ID(e2, TEST_MCP3427_DATA_READY_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(0, s_times_callback_called);

  // CH2 read: data trigger event raised immediately, callbacks called
  TEST_ASSERT_OK(mcp3427_process_event(&e1));
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_OK(mcp3427_process_event(&e2));
  TEST_ASSERT_EQUAL(2, s_times_callback_called);  // now both callbacks have been called
  TEST_ASSERT_EVENT_WITH_ID(e1, TEST_MCP3427_DATA_TRIGGER_EVENT);
  TEST_ASSERT_EVENT_WITH_ID(e2, TEST_MCP3427_DATA_TRIGGER_EVENT);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // fault callback should have never been called
  TEST_ASSERT_EQUAL(0, s_times_fault_callback_called);
}

// Test that each function gives STATUS_CODE_INVALID_ARGS with NULL storage/settings.
void test_failure_on_null(void) {
  Event e = { 0 };
  Mcp3427Settings valid_settings = {
    .sample_rate = MCP3427_SAMPLE_RATE_12_BIT,
    .conversion_mode = MCP3427_CONVERSION_MODE_ONE_SHOT,
    .addr_pin_0 = TEST_ADDR_PIN_0,
    .addr_pin_1 = TEST_ADDR_PIN_1,
    .amplifier_gain = TEST_AMP_GAIN,
    .port = TEST_I2C_PORT,
    .adc_data_ready_event = TEST_MCP3427_DATA_READY_EVENT,
    .adc_data_trigger_event = TEST_MCP3427_DATA_TRIGGER_EVENT,
  };
  Mcp3427Storage valid_storage;

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp3427_init(NULL, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp3427_init(NULL, &valid_settings));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp3427_init(&valid_storage, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp3427_register_callback(NULL, NULL, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp3427_register_callback(NULL, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mcp3427_register_callback(&valid_storage, NULL, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp3427_register_fault_callback(NULL, NULL, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mcp3427_register_fault_callback(NULL, prv_fault_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mcp3427_register_fault_callback(&valid_storage, NULL, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mcp3427_process_event(NULL));
}

// Test that we can pass unrelated events into |mcp3427_process_event|.
void test_process_event_with_unrelated_events(void) {
  Mcp3427Settings settings = {
    .sample_rate = MCP3427_SAMPLE_RATE_12_BIT,
    .conversion_mode = MCP3427_CONVERSION_MODE_ONE_SHOT,
    .addr_pin_0 = TEST_ADDR_PIN_0,
    .addr_pin_1 = TEST_ADDR_PIN_1,
    .amplifier_gain = TEST_AMP_GAIN,
    .port = TEST_I2C_PORT,
    .adc_data_ready_event = TEST_MCP3427_DATA_READY_EVENT,
    .adc_data_trigger_event = TEST_MCP3427_DATA_TRIGGER_EVENT,
  };
  Mcp3427Storage storage;
  TEST_ASSERT_OK(mcp3427_init(&storage, &settings));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  Event e = {NUM_MCP3427_TEST_EVENTS, 0};
  TEST_ASSERT_OK(mcp3427_process_event(&e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED(); // should have no effect

  e.data = 1 << 9; // would cause a segfault if lookup is done without checking
  TEST_ASSERT_OK(mcp3427_process_event(&e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // make sure there's no effect by waiting for any soft timers to expire
  delay_ms(MCP3427_MAX_CONV_TIME_MS * 2);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
