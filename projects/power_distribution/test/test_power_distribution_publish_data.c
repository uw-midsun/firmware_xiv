#include "log.h"
#include "ms_test_helpers.h"
#include "power_distribution_publish_data.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CURRENT_ID_1 1
#define TEST_CURRENT_ID_2 2
#define TEST_CURRENT_ID_3 3
#define TEST_CURRENT_DATA_1 0xFF
#define TEST_CURRENT_DATA_2 0xDA
#define TEST_CURRENT_DATA_3 0x5F

void setup_test(void) {}
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

static StatusCode prv_internal_error_callback(PowerDistributionCurrent current_id,
                                              uint16_t current_data) {
  return STATUS_CODE_INTERNAL_ERROR;
}

// Test that returning a non-OK status code from the transmitter causes publish to do the same.
void test_power_distribution_publish_data_mirrors_transmitter_error(void) {
  PowerDistributionPublishConfig config = {
    .transmitter = &prv_internal_error_callback,
    .currents_to_publish = (PowerDistributionCurrent[]){ TEST_CURRENT_ID_1 },
    .num_currents_to_publish = 1,
  };
  TEST_ASSERT_OK(power_distribution_publish_data_init(config));

  uint16_t test_current_measurements[NUM_POWER_DISTRIBUTION_CURRENTS] = {
    [TEST_CURRENT_ID_1] = TEST_CURRENT_DATA_1,
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR,
                    power_distribution_publish_data_publish(test_current_measurements));
}
