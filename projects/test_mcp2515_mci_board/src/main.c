#include "log.h"
#include "mcp2515.h"
#include "can_msg_defs.h"
#include "generic_can_mcp2515.h"
#include "generic_can.h"
#include "gpio.h"
#include "delay.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "event_queue.h"

static GenericCanMcp2515 s_can_mcp2515;

static void prv_setup_mcp2515(void) {
  Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .int_pin = { .port = GPIO_PORT_A, 8 },

    .can_bitrate = MCP2515_BITRATE_500KBPS,
    .loopback = false,
  };

  generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
}

static SystemCanDevice s_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER;
#define TEST_DLC 8
#define TEST_EXTENDED false

int main(void) {
  event_queue_init();
  gpio_init();
  gpio_it_init();
  interrupt_init();
  soft_timer_init();
  prv_setup_mcp2515();
  LOG_DEBUG("Hello, world!\n");

  uint64_t data = 0x1234567890abcdef;
  GenericCanMsg msg = {
    .id = s_id,
    .dlc = TEST_DLC,
    .extended = TEST_EXTENDED,
    .data = data
  };

  while (true) {
    LOG_DEBUG("Sending: id: %x, data: %x%x!\n", s_id, (unsigned)((data >> 32) & 0xffffffff), (unsigned)(data & 0xffffffff));
    generic_can_tx(&s_can_mcp2515.base, &msg);
    delay_ms(500);
  }
  return 0;
}


