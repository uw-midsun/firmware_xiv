#include "can.h"
#include "can_msg_defs.h"
#include "delay.h"
#include "event_queue.h"
#include "generic_can.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
#include "soft_timer.h"

#define TEST_DLC 8
#define TEST_EXTENDED false

static SystemCanDevice s_id = 123;

static GenericCanMcp2515 s_can_mcp2515;

static void prv_rx_callback(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("Received a message!\n");
  char log_message[30];
  printf("Data:\n\t");
  uint8_t i;
  for (i = 0; i < msg->dlc; i++) {
    uint8_t byte = 0;
    byte = msg->data >> (i * 8);
    printf("%x ", byte);
  }
  printf("\n");
}

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
  generic_can_register_rx(&s_can_mcp2515.base, prv_rx_callback, 0x0, 0x0, false, NULL);
}

void ps(SoftTimerId timer_id, void *context) {
  uint64_t data = 0x1234567890abcdef;
  GenericCanMsg msg = { .id = s_id, .data = data, .dlc = TEST_DLC, .extended = TEST_EXTENDED };
  LOG_DEBUG("Sending: id: %x, data: %x%x!\n", s_id, (unsigned)((data >> 32) & 0xffffffff),
            (unsigned)(data & 0xffffffff));
  generic_can_tx(&s_can_mcp2515.base, &msg);
  soft_timer_start_millis(500, ps, NULL, NULL);
}

int main(void) {
  event_queue_init();
  gpio_init();
  gpio_it_init();
  interrupt_init();
  soft_timer_init();
  prv_setup_mcp2515();
  LOG_DEBUG("Initializing mcp2515 can smoke test\n");

  Event e = { 0 };
  soft_timer_start_millis(500, ps, NULL, NULL);
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }
  return 0;
}
