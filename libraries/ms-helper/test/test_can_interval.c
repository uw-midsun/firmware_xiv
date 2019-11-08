#include "can_interval.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"
#include "event_queue.h"
#include "fsm.h"
#include "generic_can.h"
#include "generic_can_hw.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_INTERVAL_FAULT 1
#define TEST_CAN_INTERVAL_SEND_DELAY_US 100000
#define TEST_CAN_INTERVAL_PERIOD_US 1000000

static GenericCanHw s_can;

// GenericCanRxCb
static void prv_can_rx_callback(const GenericCanMsg *msg, void *context) {
  (void)msg;
  uint8_t *data = context;
  *data += 1;
  LOG_DEBUG("Callback: %u\n", *data);
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();

  const CanHwSettings can_settings = {
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  TEST_ASSERT_OK(generic_can_hw_init(&s_can, &can_settings, TEST_CAN_INTERVAL_FAULT));
  can_interval_init();
}

void teardown_test(void) {}

void test_can_interval(void) {
  GenericCan *can = (GenericCan *)&s_can;
  volatile uint8_t counter = 0;
  Event e = { 0, 0 };
  StatusCode status = NUM_STATUS_CODES;

  const GenericCanMsg msg = {
    .id = 256,
    .data = 255,
    .dlc = 1,
    .extended = true,
  };
  TEST_ASSERT_OK(generic_can_register_rx(can, prv_can_rx_callback, GENERIC_CAN_EMPTY_MASK, msg.id,
                                         true, &counter));

  CanInterval *interval = NULL;
  TEST_ASSERT_OK(can_interval_factory(can, &msg, TEST_CAN_INTERVAL_PERIOD_US, &interval));
  // First Send:
  TEST_ASSERT_OK(can_interval_enable(interval));
  // Callback is triggered.
  delay_us(TEST_CAN_INTERVAL_SEND_DELAY_US);
  TEST_ASSERT_EQUAL(1, counter);

  // Force Send
  TEST_ASSERT_OK(can_interval_send_now(interval));
  // Callback is triggered.
  delay_us(TEST_CAN_INTERVAL_SEND_DELAY_US);
  TEST_ASSERT_EQUAL(2, counter);

  // Auto Send
  delay_us(TEST_CAN_INTERVAL_PERIOD_US);
  // Callback is triggered.
  TEST_ASSERT_EQUAL(3, counter);

  // No send
  TEST_ASSERT_OK(can_interval_disable(interval));
  delay_us(TEST_CAN_INTERVAL_PERIOD_US + 1000);
  // Queue empty
  TEST_ASSERT_EQUAL(3, counter);
}
