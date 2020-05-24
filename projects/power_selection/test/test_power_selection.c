#include "adc.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "power_selection.h"
#include "power_selection_events.h"
#include "soft_timer.h"
#include "test_helpers.h"

#define TEST_CAN_DEVICE_ID 0x1

static CanStorage s_can_storage = { 0 };
static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = POWER_SELECTION_CAN_EVENT_RX,
  .tx_event = POWER_SELECTION_CAN_EVENT_TX,
  .fault_event = POWER_SELECTION_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static GpioAddress s_dcdc_address = { .port = GPIO_PORT_A, .pin = 9 };

uint16_t changeable_value = 0;
uint16_t status = 0;
uint16_t TEST_MOCK(prv_checker)() {
  LOG_DEBUG("WHY WHY WHY\n");
  return status;
}
StatusCode TEST_MOCK(adc_read_raw)(AdcChannel adc_channel, uint16_t *reading) {
  *reading = changeable_value;
  return STATUS_CODE_OK;
}

int counter = 0;
uint16_t aux_volt = 0;
uint16_t aux_temp = 0;
uint16_t dcdc_status = 0;
StatusCode prv_test_power_selection_callback_handler(const CanMessage *msg, void *context,
                                                     CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_AUX_BATTERY_STATUS, msg->msg_id);
  counter++;
  aux_volt = msg->data_u16[0];
  aux_temp = msg->data_u16[1];
  dcdc_status = msg->data_u16[2];
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  can_init(&s_can_storage, &s_can_settings);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_AUX_BATTERY_STATUS,
                          prv_test_power_selection_callback_handler, NULL);
  TEST_ASSERT_OK(aux_dcdc_monitor_init());
}

void teardown_test(void) {}

void test_power_selection_tx(void) {
  // should transmit immediately
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 1);
  changeable_value = 32;
  status = 0;

  delay_ms(1000);
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 2);
  TEST_ASSERT_EQUAL(aux_volt, (uint16_t)(changeable_value - AUX_VOLT_DEFAULT));
  TEST_ASSERT_EQUAL(aux_temp, (uint16_t)(changeable_value - AUX_TEMP_DEFAULT));
  //TEST_ASSERT_EQUAL(dcdc_status, (uint16_t)(status));
  changeable_value = 13;
  status = 20;

  delay_ms(1000);
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 3);
  TEST_ASSERT_EQUAL(aux_volt, (uint16_t)(changeable_value - AUX_VOLT_DEFAULT));
  TEST_ASSERT_EQUAL(aux_temp, (uint16_t)(changeable_value - AUX_TEMP_DEFAULT));
  //TEST_ASSERT_EQUAL(dcdc_status, (uint16_t)(status));
  changeable_value = 1;
  status = 3;

  delay_ms(100);
  TEST_ASSERT_NOT_EQUAL(counter, 4);
  TEST_ASSERT_NOT_EQUAL(aux_volt, (uint16_t)(changeable_value - AUX_VOLT_DEFAULT));
  TEST_ASSERT_NOT_EQUAL(aux_temp, (uint16_t)(changeable_value - AUX_TEMP_DEFAULT));
  //TEST_ASSERT_NOT_EQUAL(dcdc_status, (uint16_t)(status));

  delay_ms(900);
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 4);
  TEST_ASSERT_EQUAL(aux_volt, (uint16_t)(changeable_value - AUX_VOLT_DEFAULT));
  TEST_ASSERT_EQUAL(aux_temp, (uint16_t)(changeable_value - AUX_TEMP_DEFAULT));
  //TEST_ASSERT_EQUAL(dcdc_status, (uint16_t)(status));
}

void test_power_selection_rx(void) {
  CanAckRequest ack_req = { 0 };
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);

  // FOR DCDC CHECK
  gpio_set_state(&s_dcdc_address, GPIO_STATE_HIGH);
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_INVALID);

  gpio_set_state(&s_dcdc_address, GPIO_STATE_LOW);
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_OK);

  // FOR AUX CHECK
  status = 0b0101000000000000;  // UV and UT
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_INVALID);

  status = 0b1010000000000000;  // OV and OT
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_INVALID);

  status = 0b0001000000000000;  // Voltage is ok but UT
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_INVALID);

  status = 0b0010000000000000;  // Voltage is ok but OT
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_INVALID);

  status = 0b1000000000000000;  // OV but temperature is ok
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_INVALID);

  status = 0b0100000000000000;  // UV but temperature is ok
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_INVALID);

  status = 0;  // everything ok
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
  TEST_ASSERT_EQUAL(ack_req.expected_bitset, CAN_ACK_STATUS_OK);
}
