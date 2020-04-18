#include "can.h"
#include "can_transmit.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "speed_monitor.h"
#include "test_helpers.h"
#include "unity.h"
#include "watchdog.h"

typedef enum {
  TEST_CAN_EVENT_RX = 0,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS
} TestCanEvent;

#define TEST_PEDAL_RX_CAN_EVENT_DEVICE_ID 12
#define RX_TIMEOUT_MS 50

static CanStorage s_can_storage;

void setup_test(void) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_PEDAL_RX_CAN_EVENT_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  TEST_ASSERT_OK(speed_monitor_init(RX_TIMEOUT_MS));
}

void teardown_test(void) {}

void test_speed_monitor_reports_correct_thresholds(void) {
  float speed = STATIONARY_VELOCITY_THRESHOLD + 20.0;
  uint16_t speed_left = (uint16_t)speed, speed_right = (uint16_t)speed;

  TEST_ASSERT_EQUAL(*get_global_speed_state(), NUM_SPEED_STATES);

  CAN_TRANSMIT_MOTOR_VELOCITY(speed_left, speed_right);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(*get_global_speed_state(), SPEED_STATE_MOVING);

  speed = STATIONARY_VELOCITY_THRESHOLD - 20.0;
  speed_left = (uint16_t)speed, speed_right = (uint16_t)speed;
  CAN_TRANSMIT_MOTOR_VELOCITY(speed_left, speed_right);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(*get_global_speed_state(), SPEED_STATE_STATIONARY);
}

void tset_speed_monitor_has_timeout(void) {
  float speed = STATIONARY_VELOCITY_THRESHOLD + 20.0;
  uint16_t speed_left = (uint16_t)speed, speed_right = (uint16_t)speed;

  TEST_ASSERT_EQUAL(*get_global_speed_state(), NUM_SPEED_STATES);

  CAN_TRANSMIT_MOTOR_VELOCITY(speed_left, speed_right);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(*get_global_speed_state(), SPEED_STATE_MOVING);

  delay_ms(RX_TIMEOUT_MS + 5);
  TEST_ASSERT_EQUAL(*get_global_speed_state(), NUM_SPEED_STATES);

  CAN_TRANSMIT_MOTOR_VELOCITY(speed_left, speed_right);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(*get_global_speed_state(), SPEED_STATE_MOVING);
}
