#include "regen_braking.h"

#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"

#include "can.h"
#include "can_transmit.h"

#include "mci_events.h"

static CanStorage s_can_storage = { 0 };

static StatusCode prv_empty_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                         uint16_t num_remaining, void *context) {
  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
                                  MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX, MCI_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(regen_braking_init());
  // Check the regen state set to default
  TEST_ASSERT_EQUAL(REGEN_ENABLED, get_regen_braking_state());
}

void teardown_test(void) {}

void test_regen_state_off(void) {
  CanAckRequest req = { .callback = prv_empty_ack_callback,
                        .context = NULL,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER), };
  CAN_TRANSMIT_REGEN_BRAKING(&req, REGEN_DISABLED);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(REGEN_DISABLED, get_regen_braking_state());
}

void test_regen_state_on(void) {
  CanAckRequest req = { .callback = prv_empty_ack_callback,
                        .context = NULL,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER), };
  CAN_TRANSMIT_REGEN_BRAKING(&req, REGEN_ENABLED);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(REGEN_ENABLED, get_regen_braking_state());
}
