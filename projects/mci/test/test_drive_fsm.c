#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"

#include "drive_fsm.h"
#include "mci_events.h"
#include "motor_controller.h"

static CanStorage s_can_storage;
static MotorControllerStorage s_mci_storage;
static PrechargeState s_precharge_state;

void prv_setup_system_can() {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = MCI_CAN_EVENT_RX,
    .tx_event = MCI_CAN_EVENT_TX,
    .fault_event = MCI_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_can_storage, &can_settings);
}

void prv_mci_storage_init(void *context) {
  PrechargeControlSettings precharge_settings = {
    .precharge_control = { .port = GPIO_PORT_A, .pin = 9 },
    .precharge_control2 = { .port = GPIO_PORT_B, .pin = 1 },
    .precharge_monitor = { .port = GPIO_PORT_B, .pin = 0 }
  };
  precharge_control_init(&precharge_settings);
}

PrechargeState TEST_MOCK(get_precharge_state)() {
  // unpack data
  return s_precharge_state;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  prv_setup_system_can();
  prv_mci_storage_init(&s_mci_storage);
  TEST_ASSERT_OK(drive_fsm_init(&s_mci_storage));
}

void teardown_test(void) {}

static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  CanAckStatus *expected_status = context;
  TEST_ASSERT_TRUE(*expected_status == status);
  return STATUS_CODE_OK;
}

void test_neutral(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  s_precharge_state = MCI_PRECHARGE_DISCHARGED;
  CanAckStatus expected_status = CAN_ACK_STATUS_INVALID;
  CanAckRequest req = { .callback = prv_ack_callback,
                        .context = &expected_status,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_OFF);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_OFF);

  // Test that if discharged, doesn't transition to drive
  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_OFF);

  // Test that if discharged, doesn't transition to reverse
  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_REVERSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_OFF);

  // Test that if charged, will transition to drive
  expected_status = CAN_ACK_STATUS_OK;
  s_precharge_state = MCI_PRECHARGE_CHARGED;
  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_DRIVE);

  // Test that if charged, will transition to reverse
  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_OFF);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_OFF);

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_REVERSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_REVERSE);
}

void test_drive(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  s_precharge_state = MCI_PRECHARGE_CHARGED;

  // Test that if in drive, can transition to neutral
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;
  CanAckRequest req = { .callback = prv_ack_callback,
                        .context = &expected_status,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_DRIVE);

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_OFF);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_OFF);

  // Test that if in drive, can transition to reverse
  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_DRIVE);

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_REVERSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_REVERSE);
}

void test_reverse(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  // Test that if in reverse, can transition to neutral
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;
  CanAckRequest req = { .callback = prv_ack_callback,
                        .context = &expected_status,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_REVERSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_REVERSE);

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_OFF);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_OFF);

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_REVERSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_REVERSE);

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  delay_ms(10);
  TEST_ASSERT_EQUAL(drive_fsm_get_drive_state(), EE_DRIVE_OUTPUT_DRIVE);
}
