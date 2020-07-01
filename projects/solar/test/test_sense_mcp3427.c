#include "data_store.h"
#include "event_queue.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "mcp3427_adc.h"
#include "ms_test_helpers.h"
#include "sense.h"
#include "sense_mcp3427.h"
#include "soft_timer.h"
#include "solar_config.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SENSED_CH1_VALUE 0x1337
#define TEST_SENSED_CH2_VALUE 0xDEAD
#define TEST_STORED_VALUE TEST_SENSED_CH1_VALUE  // only CH1 is used

// there must be MAX_SOLAR_MCP3427 valid data points after and including this data point
#define TEST_DATA_POINT DATA_POINT_VOLTAGE_1

typedef enum TestMcp3427Event {
  TEST_MCP3427_EVENT_TRIGGER = 0,
  TEST_MCP3427_EVENT_READY,
  NUM_TEST_MCP3427_EVENTS,
} TestMcp3427Event;

// if using multiple MCP3427s, overwrite the addr pins to avoid overlap
static Mcp3427Settings s_test_mcp3427_settings = {
  .sample_rate = MCP3427_SAMPLE_RATE_12_BIT,
  .amplifier_gain = MCP3427_AMP_GAIN_1,
  .conversion_mode = MCP3427_CONVERSION_MODE_CONTINUOUS,
  .port = I2C_PORT_1,
  .addr_pin_0 = MCP3427_PIN_STATE_LOW,
  .addr_pin_1 = MCP3427_PIN_STATE_LOW,
  .adc_data_trigger_event = TEST_MCP3427_EVENT_TRIGGER,
  .adc_data_ready_event = TEST_MCP3427_EVENT_READY,
};

// all possible address combinations of MCP3427s
static struct {
  I2CPort port;
  Mcp3427PinState addr_pin_0;
  Mcp3427PinState addr_pin_1;
} s_mcp3427_address_combos[] = {
  // float & float is the same address as low & low, so we don't include it
  { I2C_PORT_1, MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_LOW },
  { I2C_PORT_1, MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_FLOAT },
  { I2C_PORT_1, MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_HIGH },
  { I2C_PORT_1, MCP3427_PIN_STATE_FLOAT, MCP3427_PIN_STATE_LOW },
  { I2C_PORT_1, MCP3427_PIN_STATE_FLOAT, MCP3427_PIN_STATE_HIGH },
  { I2C_PORT_1, MCP3427_PIN_STATE_HIGH, MCP3427_PIN_STATE_LOW },
  { I2C_PORT_1, MCP3427_PIN_STATE_HIGH, MCP3427_PIN_STATE_FLOAT },
  { I2C_PORT_1, MCP3427_PIN_STATE_HIGH, MCP3427_PIN_STATE_HIGH },
  { I2C_PORT_2, MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_LOW },
  { I2C_PORT_2, MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_FLOAT },
  { I2C_PORT_2, MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_HIGH },
  { I2C_PORT_2, MCP3427_PIN_STATE_FLOAT, MCP3427_PIN_STATE_LOW },
  { I2C_PORT_2, MCP3427_PIN_STATE_FLOAT, MCP3427_PIN_STATE_HIGH },
  { I2C_PORT_2, MCP3427_PIN_STATE_HIGH, MCP3427_PIN_STATE_LOW },
  { I2C_PORT_2, MCP3427_PIN_STATE_HIGH, MCP3427_PIN_STATE_FLOAT },
  { I2C_PORT_2, MCP3427_PIN_STATE_HIGH, MCP3427_PIN_STATE_HIGH },
};

// Helper function to get SenseMcp3427Settings that cover the max number of MCP3427s
static void prv_get_max_mcp3427s_settings(SenseMcp3427Settings *settings) {
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    settings->mcp3427s[i].mcp3427_settings = s_test_mcp3427_settings;
    settings->mcp3427s[i].mcp3427_settings.port = s_mcp3427_address_combos[i].port;
    settings->mcp3427s[i].mcp3427_settings.addr_pin_0 = s_mcp3427_address_combos[i].addr_pin_0;
    settings->mcp3427s[i].mcp3427_settings.addr_pin_1 = s_mcp3427_address_combos[i].addr_pin_1;
    settings->mcp3427s[i].data_point = TEST_DATA_POINT + i;
  }
  settings->num_mcp3427s = MAX_SOLAR_MCP3427;
}

// these is updated when |sense_register| is called
static SenseCallback s_sense_callbacks[MAX_SOLAR_MCP3427];
static void *s_sense_callback_contexts[MAX_SOLAR_MCP3427];
static uint8_t s_num_sense_callbacks;

StatusCode TEST_MOCK(sense_register)(SenseCallback callback, void *context) {
  s_sense_callbacks[s_num_sense_callbacks] = callback;
  s_sense_callback_contexts[s_num_sense_callbacks] = context;
  s_num_sense_callbacks++;
  return STATUS_CODE_OK;
}

// these are updated when the mcp3427 mocked functions are called
static Mcp3427Callback s_mcp3427_callbacks[MAX_SOLAR_MCP3427];
static void *s_mcp3427_callback_contexts[MAX_SOLAR_MCP3427];
static uint8_t s_num_mcp3427_callbacks;
static Mcp3427FaultCallback s_mcp3427_fault_callbacks[MAX_SOLAR_MCP3427];
static void *s_mcp3427_fault_callback_contexts[MAX_SOLAR_MCP3427];
static uint8_t s_num_mcp3427_fault_callbacks;
static uint8_t s_times_mcp3427_start_called;

StatusCode TEST_MOCK(mcp3427_register_callback)(Mcp3427Storage *storage, Mcp3427Callback callback,
                                                void *context) {
  s_mcp3427_callbacks[s_num_mcp3427_callbacks] = callback;
  s_mcp3427_callback_contexts[s_num_sense_callbacks] = context;
  s_num_mcp3427_callbacks++;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(mcp3427_register_fault_callback)(Mcp3427Storage *storage,
                                                      Mcp3427FaultCallback callback,
                                                      void *context) {
  s_mcp3427_fault_callbacks[s_num_mcp3427_fault_callbacks] = callback;
  s_mcp3427_fault_callback_contexts[s_num_sense_callbacks] = context;
  s_num_mcp3427_fault_callbacks++;
  return STATUS_CODE_OK;
}

// we mock this mostly to prevent actual MCP3427 cycles from starting up
StatusCode TEST_MOCK(mcp3427_start)(void) {
  s_times_mcp3427_start_called++;
  return STATUS_CODE_OK;
}

// these are updated when |data_store_set| is called
static uint8_t s_times_data_store_set_called;
static DataPoint s_data_store_set_data_point;
static uint16_t s_data_store_set_value;

// set this to set the return code of |data_store_set|
static StatusCode s_data_store_set_return_code;

StatusCode TEST_MOCK(data_store_set)(DataPoint point, uint16_t value) {
  s_times_data_store_set_called++;
  s_data_store_set_data_point = point;
  s_data_store_set_value = value;
  return s_data_store_set_return_code;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  // The dependencies on I2C and sense are mocked out, so we don't initialize them

  s_num_sense_callbacks = 0;
  s_num_mcp3427_callbacks = 0;
  s_num_mcp3427_fault_callbacks = 0;
  s_times_mcp3427_start_called = 0;
  s_times_data_store_set_called = 0;
  s_data_store_set_data_point = NUM_DATA_POINTS;  // invalid
  s_data_store_set_value = 0;
  s_data_store_set_return_code = STATUS_CODE_OK;
}
void teardown_test(void) {}

// Test that we can complete two normal sense cycles with one MCP3427.
void test_sense_mcp3427_normal_cycle_one_mcp3427(void) {
  SenseMcp3427Settings settings = {
    .mcp3427s = { {
        .data_point = TEST_DATA_POINT,
    } },
    .num_mcp3427s = 1,
  };
  settings.mcp3427s[0].mcp3427_settings = s_test_mcp3427_settings;
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));

  // make sure the various registration functions were called
  TEST_ASSERT_EQUAL(1, s_num_sense_callbacks);
  TEST_ASSERT_EQUAL(1, s_num_mcp3427_callbacks);
  TEST_ASSERT_EQUAL(1, s_num_mcp3427_fault_callbacks);
  TEST_ASSERT_EQUAL(0, s_times_mcp3427_start_called);   // |mcp3427_start| not called yet
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);  // |data_store_set| not called yet
  TEST_ASSERT_NOT_NULL(s_sense_callbacks[0]);
  TEST_ASSERT_NOT_NULL(s_mcp3427_callbacks[0]);
  TEST_ASSERT_NOT_NULL(s_mcp3427_fault_callbacks[0]);

  // start the MCP3427 cycles
  TEST_ASSERT_OK(sense_mcp3427_start());
  TEST_ASSERT_EQUAL(1, s_times_mcp3427_start_called);

  // call the MCP3427 callback, make sure nothing external happened
  s_mcp3427_callbacks[0](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                         s_mcp3427_callback_contexts[0]);
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);

  // call the sense cycle callback, make sure |data_store_set| is called
  s_sense_callbacks[0](s_sense_callback_contexts[0]);
  TEST_ASSERT_EQUAL(1, s_times_data_store_set_called);
  TEST_ASSERT_EQUAL(TEST_DATA_POINT, s_data_store_set_data_point);
  TEST_ASSERT_EQUAL(TEST_STORED_VALUE, s_data_store_set_value);

  // reset the data point and value for a second cycle
  s_data_store_set_data_point = NUM_DATA_POINTS;
  s_data_store_set_value = 0;

  // one more cycle, same thing
  s_mcp3427_callbacks[0](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                         s_mcp3427_callback_contexts[0]);
  TEST_ASSERT_EQUAL(1, s_times_data_store_set_called);
  s_sense_callbacks[0](s_sense_callback_contexts[0]);
  TEST_ASSERT_EQUAL(2, s_times_data_store_set_called);
  TEST_ASSERT_EQUAL(TEST_DATA_POINT, s_data_store_set_data_point);
  TEST_ASSERT_EQUAL(TEST_STORED_VALUE, s_data_store_set_value);

  // no fault events were raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that we can complete two normal sense cycles with the maximum number of MCP3427s.
void test_sense_mcp3427_normal_cycle_max_mcp3427s(void) {
  SenseMcp3427Settings settings;
  prv_get_max_mcp3427s_settings(&settings);
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));

  // make sure the various registration functions were called
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_num_sense_callbacks);
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_num_mcp3427_callbacks);
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_num_mcp3427_fault_callbacks);
  TEST_ASSERT_EQUAL(0, s_times_mcp3427_start_called);   // |mcp3427_start| not called yet
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);  // |data_store_set| not called yet

  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    TEST_ASSERT_NOT_NULL(s_sense_callbacks[i]);
    TEST_ASSERT_NOT_NULL(s_mcp3427_callbacks[i]);
    TEST_ASSERT_NOT_NULL(s_mcp3427_fault_callbacks[i]);
  }

  // start the MCP3427 cycles
  TEST_ASSERT_OK(sense_mcp3427_start());
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_times_mcp3427_start_called);

  // call the MCP3427 callbacks, make sure nothing external happened
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_mcp3427_callbacks[i](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                           s_mcp3427_callback_contexts[i]);
  }
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);

  // call the sense cycle callback, make sure |data_store_set| is called
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
    TEST_ASSERT_EQUAL(i + 1, s_times_data_store_set_called);
    TEST_ASSERT_EQUAL(TEST_DATA_POINT + i, s_data_store_set_data_point);
    TEST_ASSERT_EQUAL(TEST_STORED_VALUE, s_data_store_set_value);
    // reset to accurately test the next callback
    s_data_store_set_data_point = NUM_DATA_POINTS;
    s_data_store_set_value = 0;
  }

  // one more cycle, same thing
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_mcp3427_callbacks[i](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                           s_mcp3427_callback_contexts[i]);
  }
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_times_data_store_set_called);
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
    TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427 + i + 1, s_times_data_store_set_called);
    TEST_ASSERT_EQUAL(TEST_DATA_POINT + i, s_data_store_set_data_point);
    TEST_ASSERT_EQUAL(TEST_STORED_VALUE, s_data_store_set_value);
    // reset to accurately test the next callback
    s_data_store_set_data_point = NUM_DATA_POINTS;
    s_data_store_set_value = 0;
  }

  // no fault events were raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that |data_store_set| isn't called when the sense cycle is before the MCP3427 callback.
void test_sense_mcp3427_data_not_ready_max_mcp3427s(void) {
  SenseMcp3427Settings settings;
  prv_get_max_mcp3427s_settings(&settings);
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));

  // just to make sure we avoid segfaults, make sure register callbacks were called
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    TEST_ASSERT_NOT_NULL(s_sense_callbacks[i]);
    TEST_ASSERT_NOT_NULL(s_mcp3427_callbacks[i]);
  }

  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
  }
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);  // |data_store_set| wasn't called
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
  }
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);  // it still wasn't called

  TEST_ASSERT_OK(sense_mcp3427_start());
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_times_mcp3427_start_called);

  // only call the first one
  s_mcp3427_callbacks[0](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                         s_mcp3427_callback_contexts[0]);
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
  }
  TEST_ASSERT_EQUAL(1, s_times_data_store_set_called);  // only the first was called

  for (uint8_t i = 1; i < MAX_SOLAR_MCP3427; i++) {
    s_mcp3427_callbacks[i](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                           s_mcp3427_callback_contexts[i]);
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
  }
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_times_data_store_set_called);  // finally all were called
}

// Test that we log a warning when |data_store_set| returns not ok.
// We can't automatically test that a log occurs, so you have to manually verify that it occurs.
void test_sense_mcp3427_data_store_set_not_ok(void) {
  SenseMcp3427Settings settings = {
    .mcp3427s = { {
        .data_point = TEST_DATA_POINT,
    } },
    .num_mcp3427s = 1,
  };
  settings.mcp3427s[0].mcp3427_settings = s_test_mcp3427_settings;
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));

  TEST_ASSERT_NOT_NULL(s_sense_callbacks[0]);
  TEST_ASSERT_NOT_NULL(s_mcp3427_callbacks[0]);

  s_data_store_set_return_code = STATUS_CODE_INTERNAL_ERROR;
  LOG_WARN("Testing bad status code from data_store_set, there should be a warning here:\n");
  s_mcp3427_callbacks[0](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                         s_mcp3427_callback_contexts[0]);
  s_sense_callbacks[0](s_sense_callback_contexts[0]);  // warning here
  TEST_ASSERT_EQUAL(1, s_times_data_store_set_called);
}

// Test that initializing with NULL settings fails gracefully.
void test_sense_mcp3427_init_null(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_mcp3427_init(NULL));
}

// Test that initializing with too many MCP3427s fails gracefully.
void test_sense_mcp3427_init_too_many_mcp3427s(void) {
  SenseMcp3427Settings settings = { .num_mcp3427s = MAX_SOLAR_MCP3427 + 1 };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_mcp3427_init(&settings));
}
