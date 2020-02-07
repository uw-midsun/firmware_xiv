#include "pedal_rx.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "exported_enums.h"
#include "event_queue.h"
#include "interrupt.h"
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
} TestPedalCanEvent;

typedef enum {
  TEST_PEDAL_RX_RX_EVENT = 0,
  TEST_PEDAL_RX_TIMEOUT_EVENT,
} TestPedalEvent;

typedef struct {
  float throttle;
  float brake;
} TestPedalValues;

typedef struct {
  TestPedalValues* expected_value;
  PedalRxStorage* pr_storage;
  uint8_t curr_tx_test;
  bool handled_last;
  bool completed_tx;
  bool completed_watchdog_test;
  bool completed_all;
} TestPedalRxStorage;

#define TEST_PEDAL_RX_VALUE_THRESHOLD 0.01f

static CanStorage s_can_storage;
static TestPedalValues s_test_pedal_values[] = {
  [0] = { .throttle = 0.0f, .brake = 0.0f },
  [1] = { .throttle = 10.0f, .brake = 0.0f },
  [2] = { .throttle = 35.6f, .brake = 1.1f },
  [3] = { .throttle = 100.0f, .brake = 100.0f },
  [4] = { .throttle = 0.0f, .brake = 100.0f },
  [5] = { .throttle = 0.0f, .brake = 35.6f },
};

static uint32_t prv_pedal_value_to_can_msg(float pedal_value) {
  return (uint32_t)(pedal_value * PEDAL_RX_MSG_DENOMINATOR);
}

static float prv_can_msg_to_pedal_value(uint32_t can_msg) {
  return (float)(can_msg) / PEDAL_RX_MSG_DENOMINATOR;
}

static void prv_test_pedal_rx_process_event(TestPedalRxStorage* storage, Event* event) {
  TestPedalValues* expected_value = storage->expected_value;
  PedalRxStorage* pr_storage = storage->pr_storage;
  if (event->id == TEST_PEDAL_RX_RX_EVENT) {
    TEST_ASSERT_FLOAT_WITHIN(
      prv_can_msg_to_pedal_value(pr_storage->throttle),
      expected_value->throttle,
      TEST_PEDAL_RX_VALUE_THRESHOLD);
    TEST_ASSERT_FLOAT_WITHIN(
      prv_can_msg_to_pedal_value(pr_storage->brake),
      expected_value->brake,
      TEST_PEDAL_RX_VALUE_THRESHOLD);
    storage->handled_last = true;
    if (storage->completed_tx && storage->completed_watchdog_test) {
      storage->completed_all = true;
    }
  } else if (event->id == TEST_PEDAL_RX_TIMEOUT_EVENT) {
    TEST_ASSERT_TRUE(storage->completed_tx && !storage->completed_watchdog_test);
    storage->completed_watchdog_test = true;
    storage->curr_tx_test = 0;
  }
}

static void prv_transmit_test_pedal_values(SoftTimerId timer_id, void *context) {
  TestPedalRxStorage* storage = context;
  TEST_ASSERT_TRUE(storage->handled_last);
  if (storage->curr_tx_test == SIZEOF_ARRAY(s_test_pedal_values)) return;
  size_t i = storage->curr_tx_test++;
  storage->handled_last = false;
  TestPedalValues* expected_value = &s_test_pedal_values[i];
  storage->expected_value = expected_value;
  CAN_TRANSMIT_PEDAL_OUTPUT(
      prv_pedal_value_to_can_msg(expected_value->throttle),
      prv_pedal_value_to_can_msg(expected_value->brake));
  if (!storage->completed_all) {
    soft_timer_start_millis(TEST_PEDAL_RX_TX_PERIOD_MS, prv_transmit_test_pedal_values,
                            storage, NULL);
  }
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
}


void teardown_test(void) {}

void test_pedal_rx(void) {
  PedalRxSettings settings = {
    .rx_event = TEST_PEDAL_RX_RX_EVENT,
    .timeout_event = TEST_PEDAL_RX_TIMEOUT_EVENT
  };

  PedalRxStorage pr_storage = { 0 };

  TestPedalRxStorage storage = {
    .expected_value = NULL,
    .pr_storage = &pr_storage,
    .curr_tx_test = 0,
    .handled_last = true,
    .completed_tx = false,
    .completed_watchdog_test = false,
    .completed_all = false,
  };
  TEST_ASSERT_OK(pedal_rx_init(&pr_storage, &settings));

  soft_timer_start_millis(TEST_PEDAL_RX_TX_PERIOD_MS, prv_transmit_test_pedal_values,
                          &storage, NULL);
  while (!storage.completed_all) {
    Event e = { 0 };
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
      prv_test_pedal_rx_process_event(&storage, &e);
    }
  }
}
