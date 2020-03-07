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

static GenericCanMcp2515 s_can_mcp2515;

static CanStorage s_can_storage;

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
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
  return STATUS_CODE_OK;
}

void init_can(void) {
  CanSettings can_settings = {
    .device_id = 4,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  StatusCode ret = can_init(&s_can_storage, &can_settings);
  can_register_rx_default_handler(prv_rx_callback, NULL);
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
}

static SystemCanDevice s_id = 123;
#define TEST_DLC 8
#define TEST_EXTENDED false

void ps(SoftTimerId timer_id, void *context) {

  GenericCanMsg msg = {
    .id = 0x41u,
    .dlc = 4,
    .extended = false,
    .data = 0x12345678
  };

  uint64_t data = 0x1234567890abcdef;
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
  init_can();
  LOG_DEBUG("Hello, world!\n");

  uint64_t data = 0x1234567890abcdef;
  GenericCanMsg msg = { .id = s_id, .dlc = TEST_DLC, .extended = TEST_EXTENDED, .data = data };

  Event e = { 0 };

  soft_timer_start_millis(500, ps, NULL, NULL);

  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }
  return 0;
}
