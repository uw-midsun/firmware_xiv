#include "can.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "event_queue.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"

#define TEST_CAN_DEVICE_ID 0x1

static CanStorage s_can_storage;
static GenericCanMcp2515 s_generic_can = { 0 };

static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_250KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .rx_event = CHARGER_CAN_EVENT_RX,
  .tx_event = CHARGER_CAN_EVENT_TX,
  .fault_event = CHARGER_CAN_EVENT_FAULT,
  .loopback = false,
};

static Mcp2515Settings mcp2515_settings = {
  .spi_port = SPI_PORT_2,
  .spi_baudrate = 6000000,
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
  .int_pin = { .port = GPIO_PORT_A, .pin = 8 },
  .can_bitrate = MCP2515_BITRATE_250KBPS,
  .loopback = false,
};

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  can_init(&s_can_storage, &s_can_settings);
  generic_can_mcp2515_init(&s_generic_can, &mcp2515_settings);

  TEST_ASSERT_OK(charger_controller_init(&s_generic_can.base));
}

void teardown_test(void) {}

void test_charger_tx(void) {
    // should transmit immediately
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(counter, 1);

  delay_ms(100);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(counter, 2);

  delay_ms(95);
  TEST_ASSERT_EQUAL(counter, 2);

}

// void test_mcp2515_extended(void) {
//   LOG_DEBUG("Testing send extended id\n");
//   // shows that a filtered-out message does not update the static vars
//   TEST_ASSERT_OK(mcp2515_tx(&s_mcp2515, 0x19999999, true, 0xBEEFDEADBEEFDEAD, 8));
//   delay_ms(50);
//   TEST_ASSERT_NOT_EQUAL(0x19999999, s_id);
//   TEST_ASSERT_NOT_EQUAL(0xBEEFDEADBEEFDEAD, s_data);

//   // shows that a filtered-in message updates static vars
//   TEST_ASSERT_OK(mcp2515_tx(&s_mcp2515, 0x1EADBEEF, true, 0x8877665544332211, 8));
//   delay_ms(50);
//   TEST_ASSERT_EQUAL(0x1EADBEEF, s_id);
//   TEST_ASSERT_EQUAL(true, s_extended);
//   TEST_ASSERT_EQUAL(0x8877665544332211, s_data);
//   TEST_ASSERT_EQUAL(8, s_dlc);

//   delay_s(1);
//   TEST_ASSERT_EQUAL(counter, 5);
//   delay_s(1);
//   TEST_ASSERT_EQUAL(counter, 5);
// }
