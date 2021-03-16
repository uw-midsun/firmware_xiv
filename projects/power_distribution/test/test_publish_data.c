#include "can.h"
#include "can_msg_defs.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "publish_data.h"
#include "publish_data_config.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CURRENT_ID_1 1
#define TEST_CURRENT_ID_2 2
#define TEST_CURRENT_ID_3 3
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

static PowerDistributionCurrent s_single_received_current_id;
static uint16_t s_single_received_current_data;
static uint8_t s_times_single_current_callback_called;

static StatusCode prv_single_current_callback(PowerDistributionCurrent current_id,
                                              uint16_t current_data) {
  s_single_received_current_id = current_id;
  s_single_received_current_data = current_data;
  s_times_single_current_callback_called++;
  return STATUS_CODE_OK;
}

// Test that we can initialize and publish a single current
void test_power_distribution_publish_data_publish_single_current(void) {
  PowerDistributionPublishConfig config = {
    .transmitter = &prv_single_current_callback,
    .currents_to_publish = (PowerDistributionCurrent[]){ TEST_CURRENT_ID_1 },
    .num_currents_to_publish = 1,
  };
  TEST_ASSERT_OK(power_distribution_publish_data_init(config));

  s_single_received_current_id = 0;
  s_single_received_current_data = 0;
  s_times_single_current_callback_called = 0;

  uint16_t test_current_measurements[NUM_POWER_DISTRIBUTION_CURRENTS] = {
    [TEST_CURRENT_ID_1] = TEST_CURRENT_DATA_1,
  };

  // send current measurements, make sure the callback is called
  TEST_ASSERT_OK(power_distribution_publish_data_publish(test_current_measurements));
  TEST_ASSERT_EQUAL(TEST_CURRENT_ID_1, s_single_received_current_id);
  TEST_ASSERT_EQUAL(TEST_CURRENT_DATA_1, s_single_received_current_data);
  TEST_ASSERT_EQUAL(s_times_single_current_callback_called, 1);
}

#define MAX_MULTIPLE_CURRENTS 20

static PowerDistributionCurrent s_multiple_received_current_ids[MAX_MULTIPLE_CURRENTS];
static uint16_t s_multiple_received_current_data[MAX_MULTIPLE_CURRENTS];
static uint8_t s_times_multiple_callback_called;

static StatusCode prv_multiple_currents_callback(PowerDistributionCurrent current_id,
                                                 uint16_t current_data) {
  // sanity check to prevent segfaults
  if (s_times_multiple_callback_called >= MAX_MULTIPLE_CURRENTS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  s_multiple_received_current_ids[s_times_multiple_callback_called] = current_id;
  s_multiple_received_current_data[s_times_multiple_callback_called] = current_data;
  s_times_multiple_callback_called++;
  return STATUS_CODE_OK;
}

// Test that we can publish multiple currents simultaneously (and in order).
void test_power_distribution_publish_data_publish_multiple_currents(void) {
  const uint8_t num_currents = 3;

  PowerDistributionPublishConfig config = {
    .transmitter = &prv_multiple_currents_callback,
    .currents_to_publish =
        (PowerDistributionCurrent[]){
            TEST_CURRENT_ID_1,
            TEST_CURRENT_ID_2,
            TEST_CURRENT_ID_3,
        },
    .num_currents_to_publish = num_currents,
  };
  TEST_ASSERT_OK(power_distribution_publish_data_init(config));

  s_times_multiple_callback_called = 0;
  for (uint8_t i = 0; i < num_currents; i++) {
    s_multiple_received_current_ids[i] = s_multiple_received_current_data[i] = 0;
  }

  uint16_t test_current_measurements[NUM_POWER_DISTRIBUTION_CURRENTS] = {
    [TEST_CURRENT_ID_1] = TEST_CURRENT_DATA_1,
    [TEST_CURRENT_ID_2] = TEST_CURRENT_DATA_2,
    [TEST_CURRENT_ID_3] = TEST_CURRENT_DATA_3,
  };

  // send current measurements and make sure the callback is called with each current in order
  TEST_ASSERT_OK(power_distribution_publish_data_publish(test_current_measurements));
  TEST_ASSERT_EQUAL(num_currents, s_times_multiple_callback_called);
  for (uint8_t i = 0; i < num_currents; i++) {
    TEST_ASSERT_EQUAL(config.currents_to_publish[i], s_multiple_received_current_ids[i]);
    TEST_ASSERT_EQUAL(test_current_measurements[config.currents_to_publish[i]],
                      s_multiple_received_current_data[i]);
  }
}

// Test that invalid config gives an error.
void test_power_distribution_publish_data_invalid_config_errors(void) {
  // null transmitter
  PowerDistributionPublishConfig bad_config = {
    .transmitter = NULL,
    .currents_to_publish = (PowerDistributionCurrent[]){ TEST_CURRENT_ID_1 },
    .num_currents_to_publish = 1,
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, power_distribution_publish_data_init(bad_config));

  // invalid currents
  bad_config.transmitter = &prv_single_current_callback;
  bad_config.currents_to_publish[0] = NUM_POWER_DISTRIBUTION_CURRENTS;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, power_distribution_publish_data_init(bad_config));

  // null currents
  bad_config.currents_to_publish = NULL;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, power_distribution_publish_data_init(bad_config));
}

// Test that the standard configs initialize correctly.
void test_power_distribution_publish_data_standard_configs_initialize(void) {
  TEST_ASSERT_OK(
      power_distribution_publish_data_init(FRONT_POWER_DISTRIBUTION_PUBLISH_DATA_CONFIG));
  TEST_ASSERT_OK(power_distribution_publish_data_init(REAR_POWER_DISTRIBUTION_PUBLISH_DATA_CONFIG));
}

// Test that we actually send CAN messages on front.
void test_power_distribution_publish_data_send_can_msgs_front(void) {
  prv_initialize_can(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT);
  TEST_ASSERT_OK(
      power_distribution_publish_data_init(FRONT_POWER_DISTRIBUTION_PUBLISH_DATA_CONFIG));

  // the values aren't important
  uint16_t test_current_measurements[NUM_POWER_DISTRIBUTION_CURRENTS] = { 0 };
  TEST_ASSERT_OK(power_distribution_publish_data_publish(test_current_measurements));

  // make sure we can tx all of them
  for (uint16_t i = 0; i < FRONT_POWER_DISTRIBUTION_PUBLISH_DATA_CONFIG.num_currents_to_publish;
       i++) {
    MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();  // and no extras
}

// Test that we actually send CAN messages on rear.
void test_power_distribution_publish_data_send_can_msgs_rear(void) {
  prv_initialize_can(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  TEST_ASSERT_OK(power_distribution_publish_data_init(REAR_POWER_DISTRIBUTION_PUBLISH_DATA_CONFIG));

  // the values aren't important
  uint16_t test_current_measurements[NUM_POWER_DISTRIBUTION_CURRENTS] = { 0 };
  TEST_ASSERT_OK(power_distribution_publish_data_publish(test_current_measurements));

  // make sure we can tx all of them
  for (uint16_t i = 0; i < FRONT_POWER_DISTRIBUTION_PUBLISH_DATA_CONFIG.num_currents_to_publish;
       i++) {
    MS_TEST_HELPER_CAN_TX(TEST_CAN_EVENT_TX);
  }
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();  // and no extras
}
