#include "fault_tx.h"

#include <stdbool.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;

static EESolarFault s_received_fault;
static uint8_t s_received_fault_data;
static bool s_has_received_can_msg;

static StatusCode prv_handle_fault_can(const CanMessage *msg, void *context, CanAckStatus *ack) {
  CAN_UNPACK_SOLAR_FAULT(msg, (uint8_t *)&s_received_fault, &s_received_fault_data);
  s_has_received_can_msg = true;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_SOLAR, SOLAR_CAN_EVENT_TX,
                                  SOLAR_CAN_EVENT_RX, SOLAR_CAN_EVENT_FAULT);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_FAULT, prv_handle_fault_can, NULL);
  s_has_received_can_msg = false;
}
void teardown_test(void) {}

static void prv_test_single_fault(EESolarFault fault, uint8_t fault_data) {
  s_has_received_can_msg = false;
  Event fault_event = {
    .id = SOLAR_FAULT_EVENT,
    .data = FAULT_EVENT_DATA(fault, fault_data),
  };
  TEST_ASSERT_TRUE(fault_tx_process_event(&fault_event));
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_TRUE(s_has_received_can_msg);
  TEST_ASSERT_EQUAL(fault, s_received_fault);
  TEST_ASSERT_EQUAL(fault_data, s_received_fault_data);
}

void test_single_fault(void) {
  prv_test_single_fault(EE_SOLAR_FAULT_MCP3427, 0);
}

void test_all_faults(void) {
  for (EESolarFault fault = 0; fault < NUM_EE_SOLAR_FAULTS; fault++) {
    LOG_DEBUG("Testing fault %d\n", fault);
    prv_test_single_fault(fault, fault);
  }
}

void test_unknown_faults_and_events(void) {
  Event non_fault_event = { .id = SOLAR_FAULT_EVENT + 1 };
  TEST_ASSERT_FALSE(fault_tx_process_event(&non_fault_event));
  TEST_ASSERT_FALSE(s_has_received_can_msg);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  Event unknown_fault_event = {
    .id = SOLAR_FAULT_EVENT,
    .data = FAULT_EVENT_DATA(NUM_EE_SOLAR_FAULTS, 0),
  };
  TEST_ASSERT_FALSE(fault_tx_process_event(&unknown_fault_event));
  TEST_ASSERT_FALSE(s_has_received_can_msg);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  TEST_ASSERT_FALSE(fault_tx_process_event(NULL));
  TEST_ASSERT_FALSE(s_has_received_can_msg);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
