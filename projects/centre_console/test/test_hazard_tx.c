#include "hazard_tx.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "exported_enums.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static const Event s_hazard_event = { .id = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_HAZARD, .data = 0 };
static const Event s_unrecognized_event = { .id = CENTRE_CONSOLE_BUTTON_PRESS_EVENT_HAZARD + 1 };

static CanStorage s_can_storage;

static EELightState s_rx_state = NUM_EE_LIGHT_STATES;

static StatusCode prv_rx_hazard_msg(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  uint8_t rx_state = NUM_EE_LIGHT_STATES;
  StatusCode status = CAN_UNPACK_HAZARD(msg, &rx_state);
  s_rx_state = rx_state;
  return status;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                  CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX,
                                  CENTRE_CONSOLE_EVENT_CAN_FAULT);
  hazard_tx_init();

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_HAZARD, prv_rx_hazard_msg, NULL);
  s_rx_state = NUM_EE_LIGHT_STATES;
}
void teardown_test(void) {}

// Test that hazard initializes to low and sends CAN messages each change.
void test_hazard_happy_path(void) {
  // no TX of initial state upon initialization
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // starts low => transitions to high on first
  hazard_tx_process_event(&s_hazard_event);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(EE_LIGHT_STATE_ON, s_rx_state);

  // then transitions to low
  hazard_tx_process_event(&s_hazard_event);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(EE_LIGHT_STATE_OFF, s_rx_state);

  // then high again
  hazard_tx_process_event(&s_hazard_event);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(EE_LIGHT_STATE_ON, s_rx_state);

  // then low again
  hazard_tx_process_event(&s_hazard_event);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(EE_LIGHT_STATE_OFF, s_rx_state);

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that nothing changes when given NULL or non-hazard input.
void test_hazard_invalid_input(void) {
  hazard_tx_process_event(NULL);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(NUM_EE_LIGHT_STATES, s_rx_state);

  hazard_tx_process_event(&s_unrecognized_event);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(NUM_EE_LIGHT_STATES, s_rx_state);
}

// Test that |hazard_tx_process_event| returns true exactly when given a hazard event.
void test_hazard_tx_process_event_return_value(void) {
  TEST_ASSERT_TRUE(hazard_tx_process_event(&s_hazard_event));
  TEST_ASSERT_FALSE(hazard_tx_process_event(NULL));
  TEST_ASSERT_FALSE(hazard_tx_process_event(&s_unrecognized_event));
}
