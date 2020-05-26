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

static GpioAddress s_aux_status_addresses[3] = {
  { .port = GPIO_PORT_A, .pin = 0 },  // aux voltage sense
  { .port = GPIO_PORT_A, .pin = 3 },  // aux temp sense
  { .port = GPIO_PORT_A, .pin = 5 }   // aux current sense  //not needed in the module currently
};
static GpioAddress s_dcdc_address = { .port = GPIO_PORT_A, .pin = 9 };
static AdcChannel s_aux_channels[3] = { ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2 };

static uint16_t s_aux_volt_value = 0;
static uint16_t s_aux_temp_value = 0;
StatusCode TEST_MOCK(adc_read_raw)(AdcChannel adc_channel, uint16_t *reading) {
  if (adc_channel == s_aux_channels[0]) {
    *reading = s_aux_volt_value;
  } else if (adc_channel == s_aux_channels[1]) {
    *reading = s_aux_temp_value;
  }
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

static bool supposed_to_fail = false;
static StatusCode prv_can_simple_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                     uint16_t num_remaining, void *context) {
  if (supposed_to_fail) {
    TEST_ASSERT_EQUAL(status, CAN_ACK_STATUS_INVALID);
  } else {
    TEST_ASSERT_EQUAL(status, CAN_ACK_STATUS_OK);
  }
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  can_init(&s_can_storage, &s_can_settings);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_AUX_BATTERY_STATUS,
                          prv_test_power_selection_callback_handler, NULL);
  TEST_ASSERT_OK(aux_dcdc_monitor_init());
  for (int i = 0; i < 2; ++i) {
    adc_get_channel(s_aux_status_addresses[i], &s_aux_channels[i]);
  }
}

void teardown_test(void) {}

void test_power_selection_tx(void) {
  // should transmit immediately
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 1);
  s_aux_volt_value = 10;
  s_aux_temp_value = 52;

  delay_ms(1000);
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 2);
  TEST_ASSERT_EQUAL(aux_volt, (uint16_t)(s_aux_volt_value - AUX_VOLT_DEFAULT));
  TEST_ASSERT_EQUAL(aux_temp, (uint16_t)(s_aux_temp_value - AUX_TEMP_DEFAULT));
  s_aux_volt_value = 16;
  s_aux_temp_value = 62;

  delay_ms(1000);
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 3);
  TEST_ASSERT_EQUAL(aux_volt, (uint16_t)(s_aux_volt_value - AUX_VOLT_DEFAULT));
  TEST_ASSERT_EQUAL(aux_temp, (uint16_t)(s_aux_temp_value - AUX_TEMP_DEFAULT));
  s_aux_volt_value = 5;
  s_aux_temp_value = 72;

  delay_ms(100);
  TEST_ASSERT_NOT_EQUAL(counter, 4);
  TEST_ASSERT_NOT_EQUAL(aux_volt, (uint16_t)(s_aux_volt_value - AUX_VOLT_DEFAULT));
  TEST_ASSERT_NOT_EQUAL(aux_temp, (uint16_t)(s_aux_temp_value - AUX_TEMP_DEFAULT));

  delay_ms(900);
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 4);
  TEST_ASSERT_EQUAL(aux_volt, (uint16_t)(s_aux_volt_value - AUX_VOLT_DEFAULT));
  TEST_ASSERT_EQUAL(aux_temp, (uint16_t)(s_aux_temp_value - AUX_TEMP_DEFAULT));
}

void test_power_selection_rx(void) {
  CanAckRequest ack_req = { 0 };
  ack_req.callback = prv_can_simple_ack;
  MS_TEST_HELPER_CAN_TX_RX(POWER_SELECTION_CAN_EVENT_TX, POWER_SELECTION_CAN_EVENT_RX);

  // FOR DCDC CHECK
  gpio_set_state(&s_dcdc_address, GPIO_STATE_HIGH);
  supposed_to_fail = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);

  gpio_set_state(&s_dcdc_address, GPIO_STATE_LOW);
  supposed_to_fail = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC);

  // FOR AUX CHECK
  s_aux_volt_value = 5;
  s_aux_temp_value = 2;  // UV and UT
  supposed_to_fail = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);

  s_aux_volt_value = 20;
  s_aux_temp_value = 122;  // OV and OT
  supposed_to_fail = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);

  s_aux_volt_value = 11;
  s_aux_temp_value = 2;  // Voltage is ok but UT
  supposed_to_fail = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);

  s_aux_volt_value = 12;
  s_aux_temp_value = 132;  // Voltage is ok but OT
  supposed_to_fail = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);

  s_aux_volt_value = 25;
  s_aux_temp_value = 62;  // OV but temperature is ok
  supposed_to_fail = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);

  s_aux_volt_value = 3;
  s_aux_temp_value = 82;  // UV but temperature is ok
  supposed_to_fail = true;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);

  s_aux_volt_value = 12;
  s_aux_temp_value = 75;  // everything ok
  supposed_to_fail = false;
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_req, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
}
