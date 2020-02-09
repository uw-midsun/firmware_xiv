#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "delay.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "power_main_precharge_monitor.h"
#include "test_helpers.h"
#include "unity.h"

static PowerMainPrechargeMonitor s_precharge_monitor = { 0 };
static CanStorage s_can_storage;
static CanAckStatus s_can_ack_status;

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
  s_can_ack_status = NUM_CAN_ACK_STATUSES;
}

static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  LOG_DEBUG("We acked mci's message\n");
  s_can_ack_status = status;
  return STATUS_CODE_OK;
}

void teardown_test(void) {}

void test_precharge_monitor_init_will_raise_event_if_precharge_completes(void) {
  TEST_ASSERT_OK(power_main_precharge_monitor_init(&s_precharge_monitor, TEST_PRECHARGE_TIMEOUT_MS));
  TEST_ASSERT_EQUAL(s_precharge_monitor.timer_id, SOFT_TIMER_INVALID_TIMER);
  TEST_ASSERT_OK(power_main_precharge_monitor_start(&s_precharge_monitor));
  TEST_ASSERT_NOT_EQUAL(s_precharge_monitor.timer_id, SOFT_TIMER_INVALID_TIMER);

  CanAckRequest ack_request = {
    .callback = prv_ack_callback,                                                   //
    .context = NULL,                                                                //
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE),  //
  };

  delay_ms(10);
  Event e = { 0 };
  TEST_ASSERT_NOT_OK(event_process(&e));

  // motor controller sends precharge completed message
  CAN_TRANSMIT_POWER_ON_MAIN_SEQUENCE(&ack_request, EE_POWER_MAIN_SEQUENCE_PRECHARGE_COMPLETED);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // event gets raised
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_MAIN_SEQUENCE_EVENT_PRECHARGE_COMPLETE, 0);

  // ack message
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_OK(s_can_ack_status);
}

void test_precharge_monitor_times_out_after_some_time_and_raises_event(void) {
  TEST_ASSERT_OK(power_main_precharge_monitor_init(&s_precharge_monitor, TEST_PRECHARGE_TIMEOUT_MS));
  TEST_ASSERT_OK(power_main_precharge_monitor_start(&s_precharge_monitor));

  delay_ms(TEST_PRECHARGE_TIMEOUT_MS - 10);
  Event e = { 0 };
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_ms(15);
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_MAIN_SEQUENCE_EVENT_FAULT,
                              EE_POWER_MAIN_SEQUENCE_PRECHARGE_COMPLETED);
}

void test_precharge_monitor_time_out_doesnt_try(void) {
  TEST_ASSERT_OK(power_main_precharge_monitor_init(&s_precharge_monitor, TEST_PRECHARGE_TIMEOUT_MS));
  TEST_ASSERT_OK(power_main_precharge_monitor_start(&s_precharge_monitor));

  delay_ms(TEST_PRECHARGE_TIMEOUT_MS - 10);
  Event e = { 0 };
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_ms(15);
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_MAIN_SEQUENCE_EVENT_FAULT,
                              EE_POWER_MAIN_SEQUENCE_PRECHARGE_COMPLETED);
}
