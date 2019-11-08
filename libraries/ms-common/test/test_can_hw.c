#include <inttypes.h>
#include "can_hw.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static volatile size_t s_msg_rx;
static volatile uint32_t s_rx_id;
static volatile bool s_extended;
static volatile uint64_t s_rx_data;
static volatile size_t s_rx_len;

static void prv_handle_rx(void *context) {
  while (can_hw_receive(&s_rx_id, &s_extended, &s_rx_data, &s_rx_len)) {
    LOG_DEBUG("RX msg 0x%" PRIu32 " (extended %d) %zu bytes\n", s_rx_id, s_extended, s_rx_len);
    s_msg_rx++;
  }
}

static void prv_wait_rx(size_t wait_for) {
  size_t expected = s_msg_rx + wait_for;

  while (s_msg_rx != expected) {
  }
}

void setup_test(void) {
  interrupt_init();

  // Only used for delay - not needed for actual testing
  soft_timer_init();

  CanHwSettings can_settings = {
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .loopback = true,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  StatusCode ret = can_hw_init(&can_settings);
  TEST_ASSERT_OK(ret);
  ret = can_hw_register_callback(CAN_HW_EVENT_MSG_RX, prv_handle_rx, NULL);
  TEST_ASSERT_OK(ret);
  s_msg_rx = 0;
  s_rx_id = 0;
  s_extended = false;
  s_rx_data = 0;
  s_rx_len = 0;
  LOG_DEBUG("CAN initialized\n");
}

void teardown_test(void) {}

void test_can_hw_loop(void) {
  uint32_t tx_id = 0x01;
  uint64_t tx_data = 0x1122334455667788;
  size_t tx_len = 8;

  StatusCode ret = can_hw_transmit(tx_id, false, (uint8_t *)&tx_data, tx_len);
  TEST_ASSERT_OK(ret);

  prv_wait_rx(1);

  TEST_ASSERT_EQUAL(tx_id, s_rx_id);
  TEST_ASSERT_EQUAL(false, s_extended);
  TEST_ASSERT_EQUAL(tx_data, s_rx_data);
  TEST_ASSERT_EQUAL(tx_len, s_rx_len);
}

void test_can_hw_filter(void) {
  // Mask 0b11, require 0b01
  can_hw_add_filter(0x03, 0x01, false);

  // 0b0011 - fail
  StatusCode ret = can_hw_transmit(0x3, false, 0, 0);
  TEST_ASSERT_OK(ret);

  delay_ms(1);

  // 0b0101 - pass
  ret = can_hw_transmit(0x5, false, 0, 0);
  TEST_ASSERT_OK(ret);

  delay_ms(1);

  // 0b00111001 - pass
  ret = can_hw_transmit(0x39, false, 0, 0);
  TEST_ASSERT_OK(ret);

  delay_ms(1);

  // extended ID with std ID 0b1 - fail
  ret = can_hw_transmit(0x40000, true, 0, 0);
  TEST_ASSERT_OK(ret);

  delay_ms(10);

  TEST_ASSERT_EQUAL(2, s_msg_rx);
  TEST_ASSERT_EQUAL(false, s_extended);
  TEST_ASSERT_EQUAL(0x39, s_rx_id);
}

void test_can_hw_extended(void) {
  uint32_t tx_id = 0x15555555;
  uint64_t tx_data = 0x1122334455667788;
  size_t tx_len = 8;

  StatusCode ret = can_hw_transmit(tx_id, true, (uint8_t *)&tx_data, tx_len);
  TEST_ASSERT_OK(ret);

  prv_wait_rx(1);

  TEST_ASSERT_EQUAL(tx_id, s_rx_id);
  TEST_ASSERT_EQUAL(true, s_extended);
  TEST_ASSERT_EQUAL(tx_data, s_rx_data);
  TEST_ASSERT_EQUAL(tx_len, s_rx_len);
}

void test_can_hw_extended_filter(void) {
  can_hw_add_filter(0x1234567, 0x1234567, true);

  // No match extended - fail
  StatusCode ret = can_hw_transmit(0x1234547, true, 0, 0);
  TEST_ASSERT_OK(ret);

  delay_ms(1);

  // No match standard - fail
  ret = can_hw_transmit(0x123, false, 0, 0);
  TEST_ASSERT_OK(ret);

  delay_ms(1);

  // Partial invalid - fail
  ret = can_hw_transmit(0x0004567, true, 0, 0);
  TEST_ASSERT_OK(ret);

  delay_ms(1);

  // Full match - success
  ret = can_hw_transmit(0x1234567, true, 0, 0);
  TEST_ASSERT_OK(ret);

  delay_ms(10);

  TEST_ASSERT_EQUAL(1, s_msg_rx);
  TEST_ASSERT_EQUAL(true, s_extended);
  TEST_ASSERT_EQUAL(0x1234567, s_rx_id);
}
