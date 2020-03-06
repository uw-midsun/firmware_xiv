#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "precharge_monitor.h"
#include "test_helpers.h"
#include "unity.h"

static PrechargeMonitor s_precharge_monitor = { 0 };
static CanStorage s_can_storage;

static Event s_success_event = { .id = DRIVE_FSM_INPUT_EVENT_PRECHARGE_COMPLETED, .data = 0 };

static Event s_fault_event = { .id = DRIVE_FSM_INPUT_EVENT_PRECHARGE_COMPLETED, .data = 1 };

#define TEST_PRECHARGE_TIMEOUT_MS 100

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  const CanSettings s_can_settings = { .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                       .loopback = true,
                                       .bitrate = CAN_HW_BITRATE_500KBPS,
                                       .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                       .tx = { .port = GPIO_PORT_A, .pin = 12 },
                                       .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
                                       .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
                                       .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT };
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));
}

void teardown_test(void) {}

void test_precharge_monitor_init_will_raise_event_if_precharge_completes(void) {
  TEST_ASSERT_OK(precharge_monitor_init(&s_precharge_monitor, TEST_PRECHARGE_TIMEOUT_MS,
                                        &s_success_event, &s_fault_event));
  TEST_ASSERT_EQUAL(s_precharge_monitor.timer_id, SOFT_TIMER_INVALID_TIMER);
  TEST_ASSERT_OK(precharge_monitor_start(&s_precharge_monitor));
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_NOT_EQUAL(s_precharge_monitor.timer_id, SOFT_TIMER_INVALID_TIMER);

  delay_ms(10);
  Event e = { 0 };
  TEST_ASSERT_NOT_OK(event_process(&e));

  // motor controller sends precharge completed message
  CAN_TRANSMIT_PRECHARGE_COMPLETED();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // event gets raised
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_PRECHARGE_COMPLETED, 0);

  // no further event must get raised
  delay_ms(TEST_PRECHARGE_TIMEOUT_MS);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_precharge_monitor_times_out_after_some_time_and_raises_event(void) {
  TEST_ASSERT_OK(precharge_monitor_init(&s_precharge_monitor, TEST_PRECHARGE_TIMEOUT_MS,
                                        &s_success_event, &s_fault_event));
  TEST_ASSERT_OK(precharge_monitor_start(&s_precharge_monitor));
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  delay_ms(TEST_PRECHARGE_TIMEOUT_MS - 10);
  Event e = { 0 };
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_ms(15);
  MS_TEST_HELPER_ASSERT_EVENT(e, s_fault_event.id, s_fault_event.data);

  // no further event must get raised
  delay_ms(TEST_PRECHARGE_TIMEOUT_MS);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_precharge_monitor_cancel_should_cancel_before_it_faults(void) {
  TEST_ASSERT_OK(precharge_monitor_init(&s_precharge_monitor, TEST_PRECHARGE_TIMEOUT_MS,
                                        &s_success_event, &s_fault_event));
  TEST_ASSERT_OK(precharge_monitor_start(&s_precharge_monitor));
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  delay_ms(TEST_PRECHARGE_TIMEOUT_MS - 10);
  TEST_ASSERT_TRUE(precharge_monitor_cancel(&s_precharge_monitor));

  delay_ms(TEST_PRECHARGE_TIMEOUT_MS);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
