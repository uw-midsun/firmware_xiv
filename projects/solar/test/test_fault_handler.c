#include "fault_handler.h"

#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "solar_events.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SOLAR_FAULT 0
#define TEST_FAULT_DATA 0x2A

static CanStorage s_can_storage;

static uint8_t s_times_relay_opened;
static StatusCode s_relay_open_ret;

static uint8_t s_num_fault_can_msgs;
static EESolarFault s_last_can_fault;
static uint8_t s_last_can_fault_data;

StatusCode TEST_MOCK(relay_fsm_open)(void) {
  s_times_relay_opened++;
  return s_relay_open_ret;
}

static StatusCode prv_handle_fault_can_msg(const CanMessage *msg, void *context,
                                           CanAckStatus *ack_reply) {
  s_num_fault_can_msgs++;
  CAN_UNPACK_SOLAR_FAULT(msg, (uint8_t *)&s_last_can_fault, &s_last_can_fault_data);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_SOLAR, SOLAR_CAN_EVENT_TX,
                                  SOLAR_CAN_EVENT_RX, SOLAR_CAN_EVENT_FAULT);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_FAULT, prv_handle_fault_can_msg, NULL);
  s_times_relay_opened = 0;
  s_relay_open_ret = STATUS_CODE_OK;
  s_num_fault_can_msgs = 0;
  s_last_can_fault = NUM_EE_SOLAR_FAULTS;
  s_last_can_fault_data = 0;
}
void teardown_test(void) {}

// Test that raising a non-relay open fault causes the CAN message but no relay open.
void test_single_non_relay_open_fault(void) {
  FaultHandlerSettings settings = {
    .relay_open_faults = {},
    .num_relay_open_faults = 0,
  };
  TEST_ASSERT_OK(fault_handler_init(&settings));

  TEST_ASSERT_OK(fault_handler_raise_fault(TEST_SOLAR_FAULT, TEST_FAULT_DATA));
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, s_num_fault_can_msgs);
  TEST_ASSERT_EQUAL(TEST_SOLAR_FAULT, s_last_can_fault);
  TEST_ASSERT_EQUAL(TEST_FAULT_DATA, s_last_can_fault_data);
  TEST_ASSERT_EQUAL(0, s_times_relay_opened);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that raising a relay open fault causes the CAN message and a relay open.
void test_single_relay_open_fault(void) {
  FaultHandlerSettings settings = {
    .relay_open_faults = { TEST_SOLAR_FAULT },
    .num_relay_open_faults = 1,
  };
  TEST_ASSERT_OK(fault_handler_init(&settings));

  TEST_ASSERT_OK(fault_handler_raise_fault(TEST_SOLAR_FAULT, TEST_FAULT_DATA));
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, s_num_fault_can_msgs);
  TEST_ASSERT_EQUAL(TEST_SOLAR_FAULT, s_last_can_fault);
  TEST_ASSERT_EQUAL(TEST_FAULT_DATA, s_last_can_fault_data);
  TEST_ASSERT_EQUAL(1, s_times_relay_opened);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test all the valid faults, some set as relay open faults and some not.
void test_all_faults(void) {
  // the first |num_relay_open_faults| faults will be set as relay open faults
  const uint8_t num_relay_open_faults = MIN(MAX_RELAY_OPEN_FAULTS, NUM_EE_SOLAR_FAULTS) / 2;
  FaultHandlerSettings settings;
  for (EESolarFault fault = 0; fault < num_relay_open_faults; fault++) {
    settings.relay_open_faults[fault] = fault;
  }
  settings.num_relay_open_faults = num_relay_open_faults;
  TEST_ASSERT_OK(fault_handler_init(&settings));

  for (EESolarFault fault = 0; fault < NUM_EE_SOLAR_FAULTS; fault++) {
    const uint8_t data = fault + 1;
    TEST_ASSERT_OK(fault_handler_raise_fault(fault, data));
    MS_TEST_HELPER_CAN_TX_RX(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
    TEST_ASSERT_EQUAL(fault + 1, s_num_fault_can_msgs);
    TEST_ASSERT_EQUAL(fault, s_last_can_fault);
    TEST_ASSERT_EQUAL(data, s_last_can_fault_data);
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  }
  TEST_ASSERT_EQUAL(num_relay_open_faults, s_times_relay_opened);
}

// Test that |fault_handler_raise_fault| forwards the status if opening the relay fails, but we
// still send the CAN message.
void test_relay_open_failure(void) {
  FaultHandlerSettings settings = {
    .relay_open_faults = { TEST_SOLAR_FAULT },
    .num_relay_open_faults = 1,
  };
  TEST_ASSERT_OK(fault_handler_init(&settings));

  s_relay_open_ret = STATUS_CODE_INTERNAL_ERROR;
  TEST_ASSERT_EQUAL(s_relay_open_ret, fault_handler_raise_fault(TEST_SOLAR_FAULT, TEST_FAULT_DATA));
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, s_num_fault_can_msgs);
  TEST_ASSERT_EQUAL(TEST_SOLAR_FAULT, s_last_can_fault);
  TEST_ASSERT_EQUAL(TEST_FAULT_DATA, s_last_can_fault_data);
  TEST_ASSERT_EQUAL(1, s_times_relay_opened);  // it was still called
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that we fail gracefully if we try to initialize with invalid settings.
void test_invalid_initialization(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fault_handler_init(NULL));

  FaultHandlerSettings invalid_settings = {
    .num_relay_open_faults = MAX_RELAY_OPEN_FAULTS + 1,
  };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fault_handler_init(&invalid_settings));
}

// Test that raising an invalid fault fails.
void test_raising_invalid_fault(void) {
  FaultHandlerSettings settings = {
    .relay_open_faults = {},
    .num_relay_open_faults = 0,
  };
  TEST_ASSERT_OK(fault_handler_init(&settings));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, fault_handler_raise_fault(NUM_EE_SOLAR_FAULTS, 0));
}
