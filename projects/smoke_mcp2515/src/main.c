// Simple smoketest project for MCP2515 boards.

// Check with loopback mode to make sure sending and receiving messages works
// in case there is no CAN dongle
// Then switch to non-loopback mode and
// periodically transmit a message over the MCP2515, and log any
// messages recieved over the MCP2515

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

static Mcp2515Storage s_mcp2515;
static GenericCanMcp2515 s_can_mcp2515;

static uint32_t s_id_loopback = 0;
static bool s_extended = false;
static uint64_t s_data = 0;
static size_t s_dlc = 0;

static void prv_rx_callback_loopback(uint32_t id, bool extended, uint64_t data, size_t dlc,
                                     void *context) {
  s_id_loopback = id;
  s_extended = extended;
  s_data = data;
  s_dlc = dlc;
}

static void prv_rx_callback(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("Received a message!\n");
  LOG_DEBUG("Data:\n\t");
  uint8_t i;
  for (i = 0; i < msg->dlc; i++) {
    uint8_t byte = 0;
    byte = msg->data >> (i * 8);
    LOG_DEBUG("%x ", byte);
  }
  LOG_DEBUG("\n");
}

static void prv_setup_mcp2515_loopback(void) {
  Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .int_pin = { .port = GPIO_PORT_A, 8 },

    .can_bitrate = MCP2515_BITRATE_250KBPS,
    .loopback = true,
  };

  LOG_DEBUG("Initializing mcp2515 with loopback\n");
  mcp2515_init(&s_mcp2515, &mcp2515_settings);
  mcp2515_register_cbs(&s_mcp2515, prv_rx_callback_loopback, NULL, NULL);
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

  LOG_DEBUG("Initializing mcp2515 without loopback\n");
  generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
  generic_can_register_rx(&s_can_mcp2515.base, prv_rx_callback, 0x0, 0x0, false, NULL);
}

static void prv_periodic_send(SoftTimerId timer_id, void *context) {
  uint64_t data = 0x1234567890abcdef;
  GenericCanMsg msg = { .id = s_id, .data = data, .dlc = TEST_DLC, .extended = TEST_EXTENDED };
  LOG_DEBUG("Sending: id: %x, data: %x%x!\n", s_id, (unsigned)((data >> 32) & 0xffffffff),
            (unsigned)(data & 0xffffffff));
  generic_can_tx(&s_can_mcp2515.base, &msg);
  soft_timer_start_millis(500, prv_periodic_send, NULL, NULL);
}

void prv_send_messages() {
  LOG_DEBUG("Testing send standard id\n");
  mcp2515_tx(&s_mcp2515, 0x246, false, 0x1122334455667788, 8);
  delay_ms(50);

  if ((s_id_loopback == 0x246) && (s_extended == false) && (s_data == 0x1122334455667788) &&
      (s_dlc == 8)) {
    LOG_DEBUG("Standard Message Received\n");
  } else {
    LOG_DEBUG("Standard message not received\n");
  }

  LOG_DEBUG("Testing send extended id\n");
  mcp2515_tx(&s_mcp2515, 0x19999999, true, 0xBEEFDEADBEEFDEAD, 8);
  delay_ms(50);

  if ((s_id_loopback == 0x19999999) && (s_extended == true) && (s_data == 0xBEEFDEADBEEFDEAD) &&
      (s_dlc == 8)) {
    LOG_DEBUG("Extended Message Received\n");
  } else {
    LOG_DEBUG("Extended message not received\n");
  }
}

int main(void) {
  event_queue_init();
  gpio_init();
  gpio_it_init();
  interrupt_init();
  soft_timer_init();

  prv_setup_mcp2515_loopback();
  LOG_DEBUG("Initializing mcp2515 CAN smoke test with loopback\n");
  prv_send_messages();

  prv_setup_mcp2515();
  LOG_DEBUG("Initializing mcp2515 CAN smoke test without loopback\n");
  Event e = { 0 };
  soft_timer_start_millis(500, prv_periodic_send, NULL, NULL);
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }
  return 0;
}
