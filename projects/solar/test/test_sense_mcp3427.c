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
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SENSED_CH1_VALUE 0x1337
#define TEST_SENSED_CH2_VALUE 0xDEAD
#define TEST_STORED_VALUE TEST_SENSED_CH1_VALUE  // only CH1 is used

// there must be MAX_SOLAR_MCP3427 valid data points after and including this data point
#define TEST_DATA_POINT DATA_POINT_VOLTAGE_1

// Helper function to get an idx'th unique valid data point
static DataPoint prv_get_test_data_point(uint8_t idx) {
  return TEST_DATA_POINT + idx;
}

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

// Helper function to get SenseMcp3427Settings that cover the max number of MCP3427s
static void prv_get_max_mcp3427s_settings(SenseMcp3427Settings *settings) {
  const Mcp3427PinState s_base_3_digits_to_pin[3] = {
    MCP3427_PIN_STATE_LOW,
    MCP3427_PIN_STATE_FLOAT,
    MCP3427_PIN_STATE_HIGH,
  };

  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    settings->mcp3427s[i].mcp3427_settings = s_test_mcp3427_settings;
    settings->mcp3427s[i].data_point = prv_get_test_data_point(i);

    // Generate a unique port/address pin combo for each MCP3427.
    // There are 2 I2C ports and 8 address pin combos (low & low is the same as float & float).
    settings->mcp3427s[i].mcp3427_settings.port = (i < 8) ? I2C_PORT_1 : I2C_PORT_2;

    // To generate the address pins, interpret i % 8 + 1 as a 2-digit number in base 3.
    // That way we skip low & low, which corresponds to 0.
    uint8_t addr = i % 8 + 1;
    settings->mcp3427s[i].mcp3427_settings.addr_pin_0 = s_base_3_digits_to_pin[addr % 3];
    settings->mcp3427s[i].mcp3427_settings.addr_pin_1 = s_base_3_digits_to_pin[addr / 3];
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
static Mcp3427FaultCallback s_mcp3427_fault_callbacks[MAX_SOLAR_MCP3427];
static void *s_mcp3427_fault_callback_contexts[MAX_SOLAR_MCP3427];
static uint8_t s_num_mcp3427_callbacks;
static uint8_t s_times_mcp3427_start_called;

// extract the callbacks and contexts from the MCP3427 storage
StatusCode TEST_MOCK(mcp3427_start)(Mcp3427Storage *storage) {
  s_mcp3427_callbacks[s_num_mcp3427_callbacks] = storage->callback;
  s_mcp3427_callback_contexts[s_num_mcp3427_callbacks] = storage->context;
  s_mcp3427_fault_callbacks[s_num_mcp3427_callbacks] = storage->fault_callback;
  s_mcp3427_fault_callback_contexts[s_num_mcp3427_callbacks] = storage->fault_context;
  s_num_mcp3427_callbacks++;
  s_times_mcp3427_start_called++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  data_store_init();
  // The dependencies on I2C and sense are mocked out, so we don't initialize them

  s_num_sense_callbacks = 0;
  s_num_mcp3427_callbacks = 0;
  s_times_mcp3427_start_called = 0;
}
void teardown_test(void) {}

// Test that we can complete two normal sense cycles with one MCP3427.
void test_sense_mcp3427_normal_cycle_one_mcp3427(void) {
  bool is_set;
  uint16_t set_value;
  SenseMcp3427Settings settings = {
    .mcp3427s = { {
        .data_point = TEST_DATA_POINT,
    } },
    .num_mcp3427s = 1,
  };
  settings.mcp3427s[0].mcp3427_settings = s_test_mcp3427_settings;
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));
  TEST_ASSERT_EQUAL(0, s_times_mcp3427_start_called); // mcp3427 cycle not started yet

  // start the MCP3427 cycles
  TEST_ASSERT_OK(sense_mcp3427_start());
  TEST_ASSERT_EQUAL(1, s_times_mcp3427_start_called);
  data_store_get_is_set(TEST_DATA_POINT, &is_set);
  TEST_ASSERT_EQUAL(false, is_set);  // not setting data yet

  // make sure we received the various callbacks
  TEST_ASSERT_NOT_NULL(s_sense_callbacks[0]);
  TEST_ASSERT_NOT_NULL(s_mcp3427_callbacks[0]);
  TEST_ASSERT_NOT_NULL(s_mcp3427_fault_callbacks[0]);

  // call the MCP3427 callback, make sure nothing was set yet (we only set data on the sense cycle)
  s_mcp3427_callbacks[0](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                         s_mcp3427_callback_contexts[0]);
  data_store_get_is_set(TEST_DATA_POINT, &is_set);
  TEST_ASSERT_EQUAL(false, is_set);

  // call the sense cycle callback, make sure |data_store_set| is called
  s_sense_callbacks[0](s_sense_callback_contexts[0]);
  data_store_get_is_set(TEST_DATA_POINT, &is_set);
  TEST_ASSERT_EQUAL(true, is_set);
  data_store_get(TEST_DATA_POINT, &set_value);
  TEST_ASSERT_EQUAL(TEST_STORED_VALUE, set_value);

  // reset the data value for a second cycle
  data_store_set(TEST_DATA_POINT, 0);

  // one more cycle, same thing
  s_mcp3427_callbacks[0](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                         s_mcp3427_callback_contexts[0]);
  s_sense_callbacks[0](s_sense_callback_contexts[0]);
  data_store_get(TEST_DATA_POINT, &set_value);
  TEST_ASSERT_EQUAL(TEST_STORED_VALUE, set_value);

  // no fault events were raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that we can complete two normal sense cycles with the maximum number of MCP3427s.
void test_sense_mcp3427_normal_cycle_max_mcp3427s(void) {
  bool is_set;
  uint16_t set_value;
  SenseMcp3427Settings settings;
  prv_get_max_mcp3427s_settings(&settings);
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));
  TEST_ASSERT_EQUAL(0, s_times_mcp3427_start_called);  // mcp3427 cycle not started yet

  // start the MCP3427 cycles
  TEST_ASSERT_OK(sense_mcp3427_start());
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_times_mcp3427_start_called);

  // make sure we received the callbacks correctly
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    data_store_get_is_set(prv_get_test_data_point(i), &is_set);
    TEST_ASSERT_EQUAL(false, is_set);  // nothing set in the data store yet
    TEST_ASSERT_NOT_NULL(s_sense_callbacks[i]);
    TEST_ASSERT_NOT_NULL(s_mcp3427_callbacks[i]);
    TEST_ASSERT_NOT_NULL(s_mcp3427_fault_callbacks[i]);
  }

  // call the MCP3427 callbacks, make sure nothing was set in the data store
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_mcp3427_callbacks[i](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                           s_mcp3427_callback_contexts[i]);
  }
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    data_store_get_is_set(prv_get_test_data_point(i), &is_set);
    TEST_ASSERT_EQUAL(false, is_set);
  }

  // call the sense cycle callback, make sure data is set in the data store
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
    data_store_get_is_set(prv_get_test_data_point(i), &is_set);
    TEST_ASSERT_EQUAL(true, is_set);
    data_store_get(prv_get_test_data_point(i), &set_value);
    TEST_ASSERT_EQUAL(TEST_STORED_VALUE, set_value);
    // reset to accurately test the next sense cycle
    data_store_set(prv_get_test_data_point(i), 0);
  }

  // one more cycle, same thing
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_mcp3427_callbacks[i](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                           s_mcp3427_callback_contexts[i]);
  }
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
    data_store_get(prv_get_test_data_point(i), &set_value);
    TEST_ASSERT_EQUAL(TEST_STORED_VALUE, set_value);
  }

  // no fault events were raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that the data store is not set when the sense cycle is before the MCP3427 callback.
void test_sense_mcp3427_data_not_ready_max_mcp3427s(void) {
  bool is_set;
  uint16_t set_value;
  SenseMcp3427Settings settings;
  prv_get_max_mcp3427s_settings(&settings);
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));
  TEST_ASSERT_OK(sense_mcp3427_start());
  TEST_ASSERT_EQUAL(MAX_SOLAR_MCP3427, s_times_mcp3427_start_called);

  // calling the sense callbacks before the MCP3427 callbacks has no effect
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
    data_store_get_is_set(prv_get_test_data_point(i), &is_set);
    TEST_ASSERT_EQUAL(false, is_set);  // the data store was never set
  }

  // only call the first MCP3427 callback, only the first sense cycle should have an effect
  s_mcp3427_callbacks[0](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                         s_mcp3427_callback_contexts[0]);
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
    data_store_get_is_set(prv_get_test_data_point(i), &is_set);
    TEST_ASSERT_EQUAL(i == 0, is_set);  // only the first one sets the data store
  }

  // call the rest of the MCP3427 callbacks, now the data store should be called
  for (uint8_t i = 1; i < MAX_SOLAR_MCP3427; i++) {
    s_mcp3427_callbacks[i](TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE,
                           s_mcp3427_callback_contexts[i]);
    s_sense_callbacks[i](s_sense_callback_contexts[i]);
    data_store_get_is_set(prv_get_test_data_point(i), &is_set);
    TEST_ASSERT_EQUAL(true, is_set);
  }
}

// Test that a fault event is raised when an MCP3427 faults too much.
void test_sense_mcp3427_fault(void) {
  SenseMcp3427Settings settings;
  prv_get_max_mcp3427s_settings(&settings);
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));
  TEST_ASSERT_OK(sense_mcp3427_start());

  // fault enough that the next fault will cause a fault event
  for (uint8_t fault = 0; fault < MAX_CONSECUTIVE_MCP3427_FAULTS - 1; fault++) {
    for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
      s_mcp3427_fault_callbacks[i](s_mcp3427_fault_callback_contexts[i]);
      MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
    }
  }

  // then a single fault event is raised when each MCP3427 exceeds the fault limit
  // (the data is the data point associated with the MCP3427)
  Event e = { 0 };
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_mcp3427_fault_callbacks[i](s_mcp3427_fault_callback_contexts[i]);
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, SOLAR_FAULT_EVENT_MCP3427, prv_get_test_data_point(i));
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  }

  // do it again - fault events happen every MAX_CONSECUTIVE_MCP3427_FAULTS faults
  for (uint8_t fault = 0; fault < MAX_CONSECUTIVE_MCP3427_FAULTS - 1; fault++) {
    for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
      s_mcp3427_fault_callbacks[i](s_mcp3427_fault_callback_contexts[i]);
      MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
    }
  }
  for (uint8_t i = 0; i < MAX_SOLAR_MCP3427; i++) {
    s_mcp3427_fault_callbacks[i](s_mcp3427_fault_callback_contexts[i]);
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, SOLAR_FAULT_EVENT_MCP3427, prv_get_test_data_point(i));
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  }

  // do it with a single MCP3427 so we know it doesn't have to be done collectively
  for (uint8_t fault = 0; fault < MAX_CONSECUTIVE_MCP3427_FAULTS - 1; fault++) {
    s_mcp3427_fault_callbacks[0](s_mcp3427_fault_callback_contexts[0]);
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  }
  s_mcp3427_fault_callbacks[0](s_mcp3427_fault_callback_contexts[0]);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, SOLAR_FAULT_EVENT_MCP3427, prv_get_test_data_point(0));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
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
