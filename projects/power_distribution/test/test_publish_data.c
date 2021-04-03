#include "publish_data.h"

#include "can.h"
#include "can_msg_defs.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "publish_data_config.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_OUTPUT_1 1
#define TEST_OUTPUT_2 2
#define TEST_OUTPUT_3 3
#define TEST_CURRENT_DATA_1 0xFF
#define TEST_CURRENT_DATA_2 0xDA
#define TEST_CURRENT_DATA_3 0x5F

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;

static void prv_initialize_can(SystemCanDevice can_device) {
  // We don't initialize CAN in setup_test to avoid littering other tests with CAN logging
  // (and to allow setting to front or rear).
  CanSettings can_settings = {
    .device_id = can_device,
    .loopback = false,  // we don't care about receiving them
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
}

void setup_test(void) {
  interrupt_init();
  event_queue_init();
  gpio_init();
  soft_timer_init();
}
void teardown_test(void) {}

static Output s_single_received_output;
static uint16_t s_single_received_current_data;
static uint8_t s_times_single_current_callback_called;

static StatusCode prv_single_current_callback(Output output, uint16_t current_data) {
  s_single_received_output = output;
  s_single_received_current_data = current_data;
  s_times_single_current_callback_called++;
  return STATUS_CODE_OK;
}

// Test that we can initialize and publish a single current
void test_publish_data_publish_single_current(void) {
  PublishDataConfig config = {
    .transmitter = prv_single_current_callback,
    .outputs_to_publish = (Output[]){ TEST_OUTPUT_1 },
    .num_outputs_to_publish = 1,
  };
  TEST_ASSERT_OK(publish_data_init(&config));

  s_single_received_output = 0;
  s_single_received_current_data = 0;
  s_times_single_current_callback_called = 0;

  uint16_t test_current_measurements[NUM_OUTPUTS] = {
    [TEST_OUTPUT_1] = TEST_CURRENT_DATA_1,
  };

  // send current measurements, make sure the callback is called
  TEST_ASSERT_OK(publish_data_publish(test_current_measurements));
  TEST_ASSERT_EQUAL(TEST_OUTPUT_1, s_single_received_output);
  TEST_ASSERT_EQUAL(TEST_CURRENT_DATA_1, s_single_received_current_data);
  TEST_ASSERT_EQUAL(s_times_single_current_callback_called, 1);
}

#define MAX_MULTIPLE_OUTPUTS 20

static Output s_multiple_received_outputs[MAX_MULTIPLE_OUTPUTS];
static uint16_t s_multiple_received_current_data[MAX_MULTIPLE_OUTPUTS];
static uint8_t s_times_multiple_callback_called;

static StatusCode prv_multiple_currents_callback(Output output, uint16_t current_data) {
  // sanity check to prevent segfaults
  if (s_times_multiple_callback_called >= MAX_MULTIPLE_OUTPUTS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  s_multiple_received_outputs[s_times_multiple_callback_called] = output;
  s_multiple_received_current_data[s_times_multiple_callback_called] = current_data;
  s_times_multiple_callback_called++;
  return STATUS_CODE_OK;
}

// Test that we can publish multiple currents simultaneously (and in order).
void test_publish_data_publish_multiple_currents(void) {
  const uint8_t num_outputs = 3;

  PublishDataConfig config = {
    .transmitter = prv_multiple_currents_callback,
    .outputs_to_publish =
        (Output[]){
            TEST_OUTPUT_1,
            TEST_OUTPUT_2,
            TEST_OUTPUT_3,
        },
    .num_outputs_to_publish = num_outputs,
  };
  TEST_ASSERT_OK(publish_data_init(&config));

  s_times_multiple_callback_called = 0;
  for (uint8_t i = 0; i < num_outputs; i++) {
    s_multiple_received_outputs[i] = s_multiple_received_current_data[i] = 0;
  }

  uint16_t test_current_measurements[NUM_OUTPUTS] = {
    [TEST_OUTPUT_1] = TEST_CURRENT_DATA_1,
    [TEST_OUTPUT_2] = TEST_CURRENT_DATA_2,
    [TEST_OUTPUT_3] = TEST_CURRENT_DATA_3,
  };

  // send current measurements and make sure the callback is called with each output in order
  TEST_ASSERT_OK(publish_data_publish(test_current_measurements));
  TEST_ASSERT_EQUAL(num_outputs, s_times_multiple_callback_called);
  for (uint8_t i = 0; i < num_outputs; i++) {
    TEST_ASSERT_EQUAL(config.outputs_to_publish[i], s_multiple_received_outputs[i]);
    TEST_ASSERT_EQUAL(test_current_measurements[config.outputs_to_publish[i]],
                      s_multiple_received_current_data[i]);
  }
}

// Test that invalid config gives an error.
void test_power_distribution_publish_data_invalid_config_errors(void) {
  // null transmitter
  PublishDataConfig bad_config = {
    .transmitter = NULL,
    .outputs_to_publish = (Output[]){ TEST_OUTPUT_1 },
    .num_outputs_to_publish = 1,
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, publish_data_init(&bad_config));

  // invalid currents
  bad_config.transmitter = prv_single_current_callback;
  bad_config.outputs_to_publish[0] = NUM_OUTPUTS;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, publish_data_init(&bad_config));

  // null currents
  bad_config.outputs_to_publish = NULL;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, publish_data_init(&bad_config));
}

// Test that the standard configs initialize correctly.
void test_power_distribution_publish_data_standard_configs_initialize(void) {
  TEST_ASSERT_OK(publish_data_init(&g_front_publish_data_config));
  TEST_ASSERT_OK(publish_data_init(&g_rear_publish_data_config));
}

// Test that we actually send CAN messages on front.
void test_power_distribution_publish_data_send_can_msgs_front(void) {
  prv_initialize_can(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT);
  TEST_ASSERT_OK(publish_data_init(&g_front_publish_data_config));

  // the values aren't important
  uint16_t test_current_measurements[NUM_OUTPUTS] = { 0 };
  TEST_ASSERT_OK(publish_data_publish(test_current_measurements));

  // make sure we can tx all of them
  for (uint16_t i = 0; i < g_front_publish_data_config.num_outputs_to_publish; i++) {
    MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();  // and no extras
}

// Test that we actually send CAN messages on rear.
void test_power_distribution_publish_data_send_can_msgs_rear(void) {
  prv_initialize_can(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  TEST_ASSERT_OK(publish_data_init(&g_rear_publish_data_config));

  // the values aren't important
  uint16_t test_current_measurements[NUM_OUTPUTS] = { 0 };
  TEST_ASSERT_OK(publish_data_publish(test_current_measurements));

  // make sure we can tx all of them
  for (uint16_t i = 0; i < g_rear_publish_data_config.num_outputs_to_publish; i++) {
    MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();  // and no extras
}
