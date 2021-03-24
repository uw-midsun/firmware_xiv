// Test power select responses to main power on sequencing
#include "power_select_can.h"
#include "power_select.h"
#include "power_select_events.h"

#include "test_helpers.h"
#include "ms_test_helpers.h"
#include "log.h"
#include "gpio.h"
#include "interrupt.h"
#include "delay.h"
#include "string.h"

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
    .device_id = SYSTEM_CAN_DEVICE_POWER_SELECTION,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = POWER_SELECT_CAN_EVENT_RX,
    .tx_event = POWER_SELECT_CAN_EVENT_TX,
    .fault_event = POWER_SELECT_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
};

static bool s_expect_ack = false;

// make sure ACK is as expected
static StatusCode prv_test_can_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                     uint16_t num_remaining, void *context) {
  if(s_expect_ack) {
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
    s_test_valid_bitset = 0b111; // all valid

    TEST_ASSERT_OK(power_select_can_init());

    s_ack_req.callback = prv_test_can_ack;
    s_ack_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_SELECTION);
    s_ack_req.context = &s_expect_ack;
}

void teardown_test(void) {

}



void test_power_select_power_on_sequence_works(void) {
  s_expect_ack = true;

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);  
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);
}

void test_power_select_power_on_sequence_aux_fault(void) {
  // aux fault
  s_test_fault_bitset = 1 << POWER_SELECT_AUX_OVERVOLTAGE;

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // no faults
  s_test_fault_bitset = 0;

  // should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);
}

void test_power_select_power_on_sequence_aux_invalid(void) {
  // aux invalid    
  s_test_valid_bitset = 0b110;

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // change back to valid
  s_test_valid_bitset = 0b111;

  // should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);
}

// same, but for DCDC:

void test_power_select_power_on_sequence_dcdc_fault(void) {
  // dcdc fault
  s_test_fault_bitset = 1 << POWER_SELECT_DCDC_OVERVOLTAGE;

  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // no faults
  s_test_fault_bitset = 0;

  // should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);
}

void test_power_select_power_on_sequence_dcdc_invalid(void) {
  // dcdc invalid 
  s_test_valid_bitset = 0b101;

  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // change to valid
  s_test_valid_bitset = 0b111;

  // should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);
}

void test_power_select_power_on_sequence_aux_dcdc_fault(void) {
  // make sure both nack if both fault
  s_test_fault_bitset = (1 << POWER_SELECT_AUX_OVERCURRENT) | (1 << POWER_SELECT_DCDC_OVERCURRENT);

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // change to no faults  
  s_test_fault_bitset = 0;

  // should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);
}

void test_power_select_power_on_sequence_aux_dcdc_invalid(void) {
  // make sure both nack if both are invalid
  s_test_valid_bitset = 0b100;

  s_expect_ack = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  // change both back to valid
  s_test_valid_bitset = 0b111;

  // should be good now
  s_expect_ack = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);

  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&s_ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(POWER_SELECT_CAN_EVENT_TX, POWER_SELECT_CAN_EVENT_RX);
}

