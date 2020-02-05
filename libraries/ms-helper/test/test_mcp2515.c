#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "soft_timer.h"
#include "spi.h"
#include "test_helpers.h"
#include "unity.h"

static Mcp2515Storage s_mcp2515;
static volatile bool s_msg_rx = false;

static uint32_t s_id = 0;
static bool s_extended = false;
static uint64_t s_data = 0;
static size_t s_dlc = 0;

static void prv_handle_rx(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  // LOG_DEBUG("\nRX id 0x%lx (extended %d) dlc %d data 0x%lx%lx\n", id, extended, dlc,
  //           (uint32_t)(data >> 32), (uint32_t)data);

  s_id = id;
  s_extended = extended;
  s_data = data;
  s_dlc = dlc;

  s_msg_rx = true;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  s_msg_rx = false;
  s_id = 0;
  s_extended = false;
  s_data = 0;
  s_dlc = 0;

  const Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .int_pin = { .port = GPIO_PORT_A, 8 },

    .filters = {
      [MCP2515_FILTER_ID_RXF0] = { .raw = 0x246 },
      [MCP2515_FILTER_ID_RXF1] = { .raw = 0x1EADBEEF },
    },

    .loopback = true,
    .can_bitrate = MCP2515_BITRATE_250KBPS,
  };
  LOG_DEBUG("Initializing mcp2515\n");
  TEST_ASSERT_OK(mcp2515_init(&s_mcp2515, &mcp2515_settings));
  mcp2515_register_cbs(&s_mcp2515, prv_handle_rx, NULL, NULL);
}

void teardown_test(void) {}

void test_mcp2515_standard(void) {
  LOG_DEBUG("Testing send standard id\n");
  //shows that a filtered-out message does not update the static vars
  TEST_ASSERT_OK(mcp2515_tx(&s_mcp2515, 0x321, false, 0xDEADBEEFDEADBEEF, 8));
  delay_ms(50);
  TEST_ASSERT_NOT_EQUAL(0x246, s_id);
  TEST_ASSERT_NOT_EQUAL(0xDEADBEEFDEADBEEF, s_data);

  //shows that a filtered-in message updates static vars
  TEST_ASSERT_OK(mcp2515_tx(&s_mcp2515, 0x246, false, 0x1122334455667788, 8));
  delay_ms(50);
  TEST_ASSERT_EQUAL(0x246, s_id);
  TEST_ASSERT_EQUAL(false, s_extended);
  TEST_ASSERT_EQUAL(0x1122334455667788, s_data);
  TEST_ASSERT_EQUAL(8, s_dlc);
}

void test_mcp2515_extended(void) {
  LOG_DEBUG("Testing send extended id\n");
  //shows that a filtered-out message does not update the static vars
  TEST_ASSERT_OK(mcp2515_tx(&s_mcp2515, 0x19999999, true, 0xBEEFDEADBEEFDEAD, 8));
  delay_ms(50);
  TEST_ASSERT_NOT_EQUAL(0x19999999, s_id);
  TEST_ASSERT_NOT_EQUAL(0xBEEFDEADBEEFDEAD, s_data);

  //shows that a filtered-in message updates static vars
  TEST_ASSERT_OK(mcp2515_tx(&s_mcp2515, 0x1EADBEEF, true, 0x8877665544332211, 8));
  delay_ms(50);
  TEST_ASSERT_EQUAL(0x1EADBEEF, s_id);
  TEST_ASSERT_EQUAL(true, s_extended);
  TEST_ASSERT_EQUAL(0x8877665544332211, s_data);
  TEST_ASSERT_EQUAL(8, s_dlc);
}