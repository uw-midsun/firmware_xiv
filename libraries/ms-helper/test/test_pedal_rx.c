#include "pedal_rx.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_PEDAL_RX_CAN_DEVICE_ID 12
#define TEST_PEDAL_RX_TX_PERIOD_MS 100

typedef enum {
  TEST_PEDAL_RX_CAN_RX = 0,
  TEST_PEDAL_RX_CAN_TX,
  TEST_PEDAL_RX_CAN_FAULT,
  TEST_PEDAL_RX_TIMEOUT
} TestPedalCanEvent;

#define TEST_PEDAL_RX_VALUE_THRESHOLD 0.01f

static CanStorage s_can_storage;
static PedalRxStorage s_pedal_rx_storage;
static PedalRxSettings s_pedal_rx_settings = {
  .timeout_event = TEST_PEDAL_RX_TIMEOUT
};

static uint32_t prv_pedal_value_to_can_msg(float pedal_value) {
  return (uint32_t)(pedal_value * PEDAL_RX_MSG_DENOMINATOR);
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_PEDAL_RX_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = TEST_PEDAL_RX_CAN_RX,
    .tx_event = TEST_PEDAL_RX_CAN_TX,
    .fault_event = TEST_PEDAL_RX_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  TEST_ASSERT_OK(pedal_rx_init(&s_pedal_rx_storage, &s_pedal_rx_settings));
}

void teardown_test(void) {}

void test_pedal_rx_timeout_goes_off_if_pedal_messages_are_not_received_fast_enough(void) {
  uint32_t throttle_output = (uint32_t) (12.4 * EE_PEDAL_VALUE_DENOMINATOR);
  uint32_t brake_output = (uint32_t) (13.6 * EE_PEDAL_VALUE_DENOMINATOR);
  CAN_TRANSMIT_PEDAL_OUTPUT(throttle_output, brake_output);


}


void test_pedal_rx(void) {
  //PedalRxSettings settings = { .rx_event = TEST_PEDAL_RX_RX_EVENT,
  //                             .timeout_event = TEST_PEDAL_RX_TIMEOUT_EVENT };

  //PedalRxStorage pr_storage = { 0 };

  //TestPedalRxStorage storage = {
  //  .expected_value = NULL,
  //  .pr_storage = &pr_storage,
  //  .curr_tx_test = 0,
  //  .handled_last = true,
  //  .completed_tx = false,
  //  .completed_watchdog_test = false,
  //  .completed_all = false,
  //};

  //soft_timer_start_millis(TEST_PEDAL_RX_TX_PERIOD_MS, prv_transmit_test_pedal_values, &storage,
  //                        NULL);
  //TEST_ASSERT_OK(pedal_rx_init(&pr_storage, &settings));
  //while (!storage.completed_all) {
  //  Event e = { 0 };
  //  while (status_ok(event_process(&e))) {
  //    can_process_event(&e);
  //    prv_test_pedal_rx_process_event(&storage, &e);
  //  }
  //}
}
