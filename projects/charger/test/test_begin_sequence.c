#include "begin_sequence.h"
#include "can_transmit.h"
#include "charger_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"

#define TEST_CAN_DEVICE_ID 0x1
#define NUM_GPIO_SET_CALLS_IN_BEGIN_SEQUENCE 3

static uint8_t s_charger_activate_calls;
static uint8_t s_gpio_set_state_calls;
static GpioState s_gpio_get_ret;

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,         //
  .bitrate = CAN_HW_BITRATE_250KBPS,       //
  .tx = { GPIO_PORT_A, 12 },               //
  .rx = { GPIO_PORT_A, 11 },               //
  .rx_event = CHARGER_CAN_EVENT_RX,        //
  .tx_event = CHARGER_CAN_EVENT_TX,        //
  .fault_event = CHARGER_CAN_EVENT_FAULT,  //
  .loopback = true                         //
};

StatusCode TEST_MOCK(charger_controller_activate)(uint16_t max_allowable_current) {
  s_charger_activate_calls++;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *state) {
  *state = s_gpio_get_ret;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(gpio_set_state)(const GpioAddress *address, GpioState state) {
  s_gpio_set_state_calls++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);

  TEST_ASSERT_OK(begin_sequence_init());

  s_charger_activate_calls = 0;
  s_gpio_set_state_calls = 0;
  s_gpio_get_ret = GPIO_STATE_LOW;
}

void teardown_test(void) {}

void test_begin_sequence_happy_path(void) {
  // set charger 'on', so charging happens without error
  s_gpio_get_ret = GPIO_STATE_HIGH;

  event_raise(CHARGER_CHARGE_EVENT_BEGIN, 0);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);
  begin_sequence_process_event(&e);

  // tx and rx the permission request
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  TEST_ASSERT(s_charger_activate_calls == 0);
  TEST_ASSERT(s_gpio_set_state_calls == 0);

  // tx and rx the allow charging message to trigger callback
  CAN_TRANSMIT_ALLOW_CHARGING();
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  // assert correct number of function calls occured
  TEST_ASSERT(s_charger_activate_calls == 1);
  TEST_ASSERT(s_gpio_set_state_calls == NUM_GPIO_SET_CALLS_IN_BEGIN_SEQUENCE);
}

void test_begin_sequence_sad_path(void) {
  // set charger 'off', so charging errors
  s_gpio_get_ret = GPIO_STATE_LOW;

  event_raise(CHARGER_CHARGE_EVENT_BEGIN, 0);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);
  begin_sequence_process_event(&e);

  // tx and rx the permission request
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  TEST_ASSERT(s_charger_activate_calls == 0);
  TEST_ASSERT(s_gpio_set_state_calls == 0);

  // tx and rx the allow charging message to trigger callback
  CAN_TRANSMIT_ALLOW_CHARGING();
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  // assert no calls occured
  TEST_ASSERT(s_charger_activate_calls == 0);
  TEST_ASSERT(s_gpio_set_state_calls == 0);
}

void test_no_permission(void) {
  event_raise(CHARGER_CHARGE_EVENT_BEGIN, 0);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);
  begin_sequence_process_event(&e);

  // tx and rx the permission request
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  // assert nothing happens
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT(s_charger_activate_calls == 0);
  TEST_ASSERT(s_gpio_set_state_calls == 0);
}
