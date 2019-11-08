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
  LOG_DEBUG("RX id 0x%lx (extended %d) dlc %d data 0x%lx%lx\n", id, extended, dlc,
            (uint32_t)(data >> 32), (uint32_t)data);

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
    .spi_port = SPI_PORT_1,
    .baudrate = 750000,
    .mosi = { .port = GPIO_PORT_A, 7 },
    .miso = { .port = GPIO_PORT_A, 6 },
    .sclk = { .port = GPIO_PORT_A, 5 },
    .cs = { .port = GPIO_PORT_A, 4 },
    .int_pin = { .port = GPIO_PORT_A, 3 },

    .loopback = true,
    .rx_cb = prv_handle_rx,
    .context = NULL,
  };
  TEST_ASSERT_OK(mcp2515_init(&s_mcp2515, &mcp2515_settings));
}

void teardown_test(void) {}

void test_mcp2515_standard(void) {
  TEST_ASSERT_OK(mcp2515_tx(&s_mcp2515, 0x123, false, 0x1122334455667788, 8));

  while (!s_msg_rx) {
  }

  TEST_ASSERT_EQUAL(0x123, s_id);
  TEST_ASSERT_EQUAL(false, s_extended);
  TEST_ASSERT_EQUAL(0x1122334455667788, s_data);
  TEST_ASSERT_EQUAL(8, s_dlc);
}

void test_mcp2515_extended(void) {
  TEST_ASSERT_OK(mcp2515_tx(&s_mcp2515, 0x15555555, true, 0x8866442200, 5));

  while (!s_msg_rx) {
  }

  TEST_ASSERT_EQUAL(0x15555555, s_id);
  TEST_ASSERT_EQUAL(true, s_extended);
  TEST_ASSERT_EQUAL(0x8866442200, s_data);
  TEST_ASSERT_EQUAL(5, s_dlc);
}
