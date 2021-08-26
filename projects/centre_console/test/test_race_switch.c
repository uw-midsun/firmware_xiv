#include <string.h>

#include "centre_console_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "race_switch.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"
#include "log.h"

static CanStorage s_can_storage = { 0 };

static RaceSwitchFsmStorage s_race_switch_fsm_storage;
static GpioState s_returned_state;
static GpioAddress s_race_switch_address = { .port = GPIO_PORT_A, .pin = 4 };
static GpioAddress s_voltage_monitor_address = { .port = GPIO_PORT_B, .pin = 7 };
static uint8_t s_times_callback_called;
static CanAckStatus s_ack_reply_value;

StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *state) {
  *state = s_returned_state;
  return STATUS_CODE_OK;
}

// static StatusCode prv_regen_callback(const CanMessage *msg, void *context,
//                                      CanAckStatus *ack_reply) {
//   *ack_reply = s_ack_reply_value;
//   s_times_callback_called++;
//   return STATUS_CODE_OK;
// }

static bool prv_process_fsm_event_manually(void) {
  Event e;
  event_process(&e);
  bool transitioned = race_switch_fsm_process_event(&s_race_switch_fsm_storage, &e);
  return transitioned;
}

void setup_test(void) {
  s_times_callback_called = 0;
  interrupt_init();
  gpio_init();
  gpio_it_init();
  event_queue_init();
  soft_timer_init();
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                  CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX,
                                  CENTRE_CONSOLE_EVENT_CAN_FAULT);
  // TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_RACE_NORMAL_SWITCH_MODE,
  //                                        prv_regen_callback, NULL));
  //MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  memset(&s_race_switch_fsm_storage, 0, sizeof(s_race_switch_fsm_storage));
  TEST_ASSERT_OK(race_switch_fsm_init(&s_race_switch_fsm_storage));
  //MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  // process race switch on FSM
  //MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  // process race switch off FSM
  //MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  //LOG_DEBUG("times callback called: %u", s_times_callback_called);
}

void teardown_test(void) {}

void prv_assert_current_race_state(RaceState state) {
  TEST_ASSERT_EQUAL(state, race_switch_fsm_get_current_state(&s_race_switch_fsm_storage));
}

void test_transition_to_race(void) {
  // Test state machine when normal -> race
  // Initially the module begins in normal mode so PA4 is low
  s_returned_state = GPIO_STATE_LOW;
  prv_assert_current_race_state(RACE_STATE_OFF);

  // Mock rising edge on race switch pin
  s_returned_state = GPIO_STATE_HIGH;

  // Trigger interrupt to change fsm state from normal to race
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  //MS_TEST_HELPER_CAN_RX(CENTRE_CONSOLE_EVENT_CAN_RX);
  //LOG_DEBUG("times callback called: %u", s_times_callback_called);
  TEST_ASSERT_TRUE(prv_process_fsm_event_manually());
  //MS_TEST_HELPER_CAN_TX(CENTRE_CONSOLE_EVENT_CAN_TX);
  //MS_TEST_HELPER_CAN_RX(CENTRE_CONSOLE_EVENT_CAN_RX);
  prv_assert_current_race_state(RACE_STATE_ON);

  // Test if no error when interrupt is triggered with same edge multiple times
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));

  // No fsm state transition will occur so race_switch_fsm_process_event will return false
  TEST_ASSERT_FALSE(prv_process_fsm_event_manually());
  prv_assert_current_race_state(RACE_STATE_ON);
}

void test_transition_to_normal(void) {
  // Test state machine when race -> normal
  // Initially the module begins in race mode so PA4 is high
  s_returned_state = GPIO_STATE_HIGH;
  prv_assert_current_race_state(RACE_STATE_ON);

  // Mock falling edge on race switch pin
  s_returned_state = GPIO_STATE_LOW;

  // Trigger interrupt to change fsm state from race to normal
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  //MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(prv_process_fsm_event_manually());
  prv_assert_current_race_state(RACE_STATE_OFF);
}

void test_voltage_during_transition(void) {
  // Test voltage regulator when normal -> race -> normal
  // Initially the car is in normal mode so the voltage regulator is enabled
  s_returned_state = GPIO_STATE_LOW;
  prv_assert_current_race_state(RACE_STATE_OFF);

  GpioState voltage_monitor_state;
  // Mock high state on voltage monitor pin
  s_returned_state = GPIO_STATE_HIGH;

  gpio_get_state(&s_voltage_monitor_address, &voltage_monitor_state);
  TEST_ASSERT_EQUAL(voltage_monitor_state, s_returned_state);

  // Switch to race mode
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  //MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(prv_process_fsm_event_manually());
  prv_assert_current_race_state(RACE_STATE_ON);

  // Mock low state on voltage monitor pin
  s_returned_state = GPIO_STATE_LOW;
  gpio_get_state(&s_voltage_monitor_address, &voltage_monitor_state);
  TEST_ASSERT_EQUAL(voltage_monitor_state, s_returned_state);

  // Switch to normal mode
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  //MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(prv_process_fsm_event_manually());
  prv_assert_current_race_state(RACE_STATE_OFF);

  // Mock high state on voltage monitor pin
  s_returned_state = GPIO_STATE_HIGH;
  gpio_get_state(&s_voltage_monitor_address, &voltage_monitor_state);
  TEST_ASSERT_EQUAL(voltage_monitor_state, s_returned_state);
}
