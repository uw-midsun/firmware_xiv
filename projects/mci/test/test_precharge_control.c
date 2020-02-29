#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "delay.h"
#include "exported_enums.h"
#include "test_helpers.h"
#include "ms_test_helpers.h"
#include "interrupt.h"
#include "gpio.h"
#include "gpio_it.h"
#include "string.h"
#include "log.h"

#include "mci_events.h"
#include "precharge_control.h"

static CanStorage s_can_storage;

static bool s_precharge_completed = false;

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

static StatusCode prv_precharge_completed(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  s_precharge_completed = true;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  gpio_it_init();
  event_queue_init();
  prv_setup_system_can();
  PrechargeControlStorage *storage = test_get_storage();
  memset(storage, 0, sizeof(PrechargeControlStorage));
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_PRECHARGE_COMPLETED, prv_precharge_completed, NULL);
  s_precharge_completed = false;
}

void teardown_test(void) {}

static PrechargeControlSettings s_settings = { 
  .precharge_control = {.port = GPIO_PORT_A, .pin = 1 },
  .precharge_control2 = {.port = GPIO_PORT_A, .pin = 2 },
  .precharge_monitor = {.port = GPIO_PORT_B, .pin = 1 }
};


void test_precharge_initializing_twice_returns_non_ok_status() {
  TEST_ASSERT_OK(precharge_control_init(&s_settings));
  TEST_ASSERT_NOT_OK(precharge_control_init(&s_settings));
}

void test_precharge_begins_precharge_process_when_can_msg_is_received(void) {
  TEST_ASSERT_OK(precharge_control_init(&s_settings));
  CAN_TRANSMIT_BEGIN_PRECHARGE();
  TEST_ASSERT_EQUAL(get_precharge_state(), MCI_PRECHARGE_DISCHARGED);

  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);

  GpioState state =  NUM_GPIO_STATES;

  TEST_ASSERT_OK(gpio_get_state(&s_settings.precharge_control, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);

  state =  NUM_GPIO_STATES;
  TEST_ASSERT_OK(gpio_get_state(&s_settings.precharge_control2, &state));
  TEST_ASSERT_EQUAL(state, GPIO_STATE_HIGH);

  // Should still be in discharged state cuz interrupt hasn't triggered.
  TEST_ASSERT_EQUAL(get_precharge_state(), MCI_PRECHARGE_DISCHARGED);
}

void test_precharge_sends_can_message_when_precharge_has_completed(void) {
  TEST_ASSERT_OK(precharge_control_init(&s_settings));
  CAN_TRANSMIT_BEGIN_PRECHARGE();
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);

  gpio_it_trigger_interrupt(&s_settings.precharge_monitor);
  TEST_ASSERT_EQUAL(get_precharge_state(), MCI_PRECHARGE_CHARGED);

  TEST_ASSERT_FALSE(s_precharge_completed);
  MS_TEST_HELPER_CAN_TX_RX(MCI_CAN_EVENT_TX, MCI_CAN_EVENT_RX);
  TEST_ASSERT_TRUE(s_precharge_completed);
}

void test_precharge_clears_precharge_whe(void) {
  return;
}




//void test_run(void) {
//  return;
//  //MotorControllerStorage *storage = &s_mci_storage;
//  //TEST_ASSERT_TRUE(get_precharge_state(storage) == GPIO_STATE_LOW);
//
//  //// Test that a non precharge power main sequence message does nothing
//  //CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(NULL, EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS);
//  //// TODO(SOFT-113): Check if this is valid amount of time to wait,
//  //// i.e. how long does precharge take
//  //delay_ms(500);
//  //TEST_ASSERT_TRUE(storage->precharge_storage.state != MCI_PRECHARGE_CHARGED);
//
//  //// Test that a precharge message precharges
//  //CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(NULL, EE_POWER_MAIN_SEQUENCE_BEGIN_PRECHARGE);
//  //delay_ms(500);
//  //TEST_ASSERT_TRUE(storage->precharge_storage.state == MCI_PRECHARGE_CHARGED);
//  //// TODO(SOFT-113): add a check to ensure proper discharge
//}
