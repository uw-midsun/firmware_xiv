#include "generic_can_hw.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_hw.h"
#include "delay.h"
#include "event_queue.h"
#include "fsm.h"
#include "generic_can.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_GENERIC_CAN_HW_FAULT_EVENT 1

static GenericCanHw s_can;

// GenericCanRxCb
static void prv_can_rx_callback(const GenericCanMsg *msg, void *context) {
  (void)msg;
  uint8_t *data = context;
  *data += 1;
  LOG_DEBUG("Callback\n");
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();

  const CanHwSettings can_hw_settings = {
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  TEST_ASSERT_OK(generic_can_hw_init(&s_can, &can_hw_settings, TEST_GENERIC_CAN_HW_FAULT_EVENT));
}

void teardown_test(void) {}

void test_generic_can(void) {
  GenericCan *can = (GenericCan *)&s_can;

  volatile uint8_t counter = 0;

  GenericCanMsg msg = {
    .id = 0x0000FF,
    .data = 255,
    .dlc = 1,
    .extended = true,
  };

  TEST_ASSERT_OK(generic_can_register_rx(can, prv_can_rx_callback, GENERIC_CAN_EMPTY_MASK, msg.id,
                                         true, &counter));

  Event e = { 0, 0 };
  StatusCode status = NUM_STATUS_CODES;
  // TX
  TEST_ASSERT_OK(generic_can_tx(can, &msg));
  // RX
  delay_ms(300);
  // Callback is triggered.
  TEST_ASSERT_EQUAL(1, counter);

  // TX (Masked)
  --msg.id;
  TEST_ASSERT_OK(generic_can_tx(can, &msg));
  // RX
  delay_ms(300);
  // Callback is triggered.
  TEST_ASSERT_EQUAL(1, counter);
}
