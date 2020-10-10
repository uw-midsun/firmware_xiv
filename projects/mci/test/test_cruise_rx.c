#include "cruise_rx.h"

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
#include "precharge_control.h"

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

PrechargeState TEST_MOCK(get_precharge_state)() {
  // unpack data
  return MCI_PRECHARGE_CHARGED;
}

static StatusCode prv_empty_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                         uint16_t num_remaining, void *context) {
  return STATUS_CODE_OK;
}

static void prv_reset_test_state() {
  s_precharge_state = MCI_PRECHARGE_DISCHARGED;
  CanAckRequest req = { .callback = prv_empty_ack_callback,
                        .context = NULL,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  LOG_DEBUG("Ensuring everything is off.\n");
  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_OFF);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_OFF, drive_fsm_get_drive_state());
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  prv_setup_system_can();
  drive_fsm_init();
  prv_reset_test_state();
  cruise_rx_init();
}

void teardown_test(void) {
  prv_reset_test_state();
}

void test_enter_cruise_fail(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  CanAckRequest req = { .callback = prv_empty_ack_callback,
                        .context = NULL,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_OFF, drive_fsm_get_drive_state());

  // Test entering cruise
  LOG_DEBUG("Should fail to enter crusie if off.");
  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_OFF, drive_fsm_get_drive_state());
  TEST_ASSERT_FALSE(drive_fsm_is_cruise());

  LOG_DEBUG("Should fail to enter crusie if in reverse.");
  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_REVERSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_REVERSE, drive_fsm_get_drive_state());

  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_REVERSE, drive_fsm_get_drive_state());
  TEST_ASSERT_FALSE(drive_fsm_is_cruise());

  LOG_DEBUG("Should fail to enter crusie if under speed.");
  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_REVERSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_REVERSE, drive_fsm_get_drive_state());

  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_REVERSE, drive_fsm_get_drive_state());
  TEST_ASSERT_FALSE(drive_fsm_is_cruise());
}

void test_toggle_cruise_manual(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  CanAckRequest req = { .callback = prv_empty_ack_callback,
                        .context = NULL,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  cruise_rx_update_velocity(MCI_MIN_CRUISE_VELOCITY_MS + 2.0f);

  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  TEST_ASSERT_TRUE(drive_fsm_is_cruise());

  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  TEST_ASSERT_FALSE(drive_fsm_is_cruise());
}

void test_exit_cruise_reverse(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  CanAckRequest req = { .callback = prv_empty_ack_callback,
                        .context = NULL,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  cruise_rx_update_velocity(MCI_MIN_CRUISE_VELOCITY_MS + 2.0f);

  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  TEST_ASSERT_TRUE(drive_fsm_is_cruise());

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_REVERSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_REVERSE, drive_fsm_get_drive_state());
  TEST_ASSERT_FALSE(drive_fsm_is_cruise());
}

void test_exit_cruise_off(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  CanAckRequest req = { .callback = prv_empty_ack_callback,
                        .context = NULL,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  cruise_rx_update_velocity(MCI_MIN_CRUISE_VELOCITY_MS + 2.0f);

  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  TEST_ASSERT_TRUE(drive_fsm_is_cruise());

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_OFF);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_OFF, drive_fsm_get_drive_state());
  TEST_ASSERT_FALSE(drive_fsm_is_cruise());
}

void test_cruise_velocity(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  CanAckRequest req = { .callback = prv_empty_ack_callback,
                        .context = NULL,
                        .expected_bitset =
                            CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };

  CAN_TRANSMIT_DRIVE_OUTPUT(&req, EE_DRIVE_OUTPUT_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  float expected_cruise_velocity = MCI_MIN_CRUISE_VELOCITY_MS + 2.0f;
  cruise_rx_update_velocity(expected_cruise_velocity);

  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_TOGGLE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(EE_DRIVE_OUTPUT_DRIVE, drive_fsm_get_drive_state());
  TEST_ASSERT_TRUE(drive_fsm_is_cruise());
  TEST_ASSERT_EQUAL(expected_cruise_velocity,  cruise_rx_get_target_velocity());
  
  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_INCREASE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  expected_cruise_velocity += MCI_CRUISE_CHANGE_AMOUNT_MS;
  TEST_ASSERT_EQUAL(expected_cruise_velocity,  cruise_rx_get_target_velocity());
  
  
  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_DECREASE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  expected_cruise_velocity -=  MCI_CRUISE_CHANGE_AMOUNT_MS;
  TEST_ASSERT_EQUAL(expected_cruise_velocity,  cruise_rx_get_target_velocity());
  
  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_DECREASE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  expected_cruise_velocity -=  MCI_CRUISE_CHANGE_AMOUNT_MS;
  TEST_ASSERT_EQUAL(expected_cruise_velocity,  cruise_rx_get_target_velocity());
}

void test_cruise_velocity_fail(void) {
  MotorControllerStorage *storage = &s_mci_storage;
  float expected_cruise_velocity = MCI_MIN_CRUISE_VELOCITY_MS + 2.0f;
  cruise_rx_update_velocity(expected_cruise_velocity);
  
  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_INCREASE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(expected_cruise_velocity,  cruise_rx_get_target_velocity());
  
  
  CAN_TRANSMIT_CRUISE_CONTROL_COMMAND(EE_CRUISE_CONTROL_COMMAND_DECREASE);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(expected_cruise_velocity,  cruise_rx_get_target_velocity());
}