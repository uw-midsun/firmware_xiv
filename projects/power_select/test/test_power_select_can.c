// Test power select responses to main power on sequencing
#include "power_select.h"
#include "power_select_can.h"
#include "power_select_events.h"

#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "string.h"
#include "test_helpers.h"

static uint16_t s_test_fault_bitset;

uint16_t TEST_MOCK(power_select_get_fault_bitset)(void) {
  return s_test_fault_bitset;
}

static uint8_t s_test_valid_bitset;

uint8_t TEST_MOCK(power_select_get_valid_bitset)(void) {
  return s_test_valid_bitset;
}

static CanStorage s_can_storage = { 0 };

static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_POWER_SELECT,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = POWER_SELECT_CAN_EVENT_RX,
  .tx_event = POWER_SELECT_CAN_EVENT_TX,
  .fault_event = POWER_SELECT_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static bool s_expect_ack = false;
static uint16_t s_times_ack_called;

// Make sure ACK is as expected
static StatusCode prv_test_can_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  s_times_ack_called++;
  if (s_expect_ack) {
    TEST_ASSERT_EQUAL(CAN_ACK_STATUS_OK, status);
  } else {
    TEST_ASSERT_NOT_EQUAL(CAN_ACK_STATUS_OK, status);
  }
  return STATUS_CODE_OK;
}

static CanAckRequest s_ack_req;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);

  s_test_fault_bitset = 0;
  s_test_valid_bitset = 0b111;  // all valid

  TEST_ASSERT_OK(power_select_can_init());

  s_ack_req.callback = prv_test_can_ack;
  s_ack_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_SELECT);
  s_ack_req.context = &s_expect_ack;

  s_times_ack_called = 0;
}

void teardown_test(void) {}

void test_power_on_main_sequence_works(void) {
  s_expect_ack = true;

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(2, s_times_ack_called);
}

void test_power_on_main_sequence_aux_fault(void) {
  // Aux fault
  s_test_fault_bitset = 1 << POWER_SELECT_AUX_OVERVOLTAGE;

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // No faults
  s_test_fault_bitset = 0;

  // Should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_ack_called);
}

void test_power_on_main_sequence_aux_invalid(void) {
  // Aux invalid
  s_test_valid_bitset = 0b110;

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // Change back to valid
  s_test_valid_bitset = 0b111;

  // Should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_ack_called);
}

// Same, but for DCDC:
void test_power_on_main_sequence_dcdc_fault(void) {
  // DCDC fault
  s_test_fault_bitset = 1 << POWER_SELECT_DCDC_OVERVOLTAGE;

  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // No faults
  s_test_fault_bitset = 0;

  // Should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_ack_called);
}

void test_power_on_main_sequence_dcdc_invalid(void) {
  // DCDC invalid
  s_test_valid_bitset = 0b101;

  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // Change to valid
  s_test_valid_bitset = 0b111;

  // Should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_ack_called);
}

void test_power_on_main_sequence_aux_dcdc_fault(void) {
  // Make sure both nack if both fault
  s_test_fault_bitset = (1 << POWER_SELECT_AUX_OVERCURRENT) | (1 << POWER_SELECT_DCDC_OVERCURRENT);

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // Change to no faults
  s_test_fault_bitset = 0;

  // Should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_ack_called);
}

void test_power_on_main_sequence_aux_dcdc_invalid(void) {
  // Make sure both nack if both are invalid
  s_test_valid_bitset = 0b100;

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // Change both back to valid
  s_test_valid_bitset = 0b111;

  // Should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(4, s_times_ack_called);
}

void test_power_on_aux_sequence_works(void) {
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_ack_called);
}

// Make sure DCDC fault/invalid doesn't affect aux sequence acks
void test_power_on_aux_sequence_works_without_dcdc(void) {
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // DCDC fault
  s_test_fault_bitset |= 1 << POWER_SELECT_DCDC_OVERCURRENT;
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // No fault, DCDC invalid
  s_test_fault_bitset &= ~(1 << POWER_SELECT_DCDC_OVERCURRENT);
  s_test_valid_bitset &= ~(1 << POWER_SELECT_DCDC);
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_ack_called);
}

void test_power_on_aux_sequence_aux_fault(void) {
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // Aux fault
  s_expect_ack = false;
  s_test_fault_bitset |= 1 << POWER_SELECT_AUX_OVERCURRENT;

  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_test_fault_bitset &= ~(1 << POWER_SELECT_AUX_OVERCURRENT);

  // Should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_ack_called);
}

void test_power_on_aux_sequence_aux_invalid(void) {
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // Aux fault
  s_expect_ack = false;
  s_test_valid_bitset &= ~(1 << POWER_SELECT_AUX);

  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_test_valid_bitset |= 1 << POWER_SELECT_AUX;

  // Should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&s_ack_req, EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS); 
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(3, s_times_ack_called);
}
