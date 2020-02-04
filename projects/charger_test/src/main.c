#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "gpio_it.h"
#include "delay.h"       // For real-time delays
#include "gpio.h"        // General Purpose I/O control.
#include "interrupt.h"   // For enabling interrupts.
#include "misc.h"        // Various helper functions/macros.
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "spi.h"
#include "mcp2515.h"

GpioAddress connection_address = { .port = GPIO_PORT_B, .pin = 1 };
GpioAddress receive_interrupt_address = { .port = GPIO_PORT_A, .pin = 8 };

static Mcp2515Storage s_mcp_storage = { 0 };

static void rx_message_callback(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  //uint8_t data1, data2, data3, data4, data5, data6, data7, data8;
  //data1 = data >> 56;
  //data2 = data >> 48;
  //data3 = data >> 40;
  //data4 = data >> 32;
  //data5 = data >> 24;
  //data6 = data >> 16;
  //data7 = data >> 8;
  //data8 = data && 0xff;
  LOG_DEBUG("message received:\nid: %lx\textended: %d\tdlc: %d\n", id, extended, dlc);
  //LOG_DEBUG("data: %x %x %x\n", data1, data2, data3);
  LOG_DEBUG("msb: %lx\n", (uint32_t) (data >> 32));
  LOG_DEBUG("lsb: %lx\n", (uint32_t) (data & 0xffffffff));
  
}

void periodic_tx(SoftTimerId id, void* context) {
  mcp2515_tx(&s_mcp_storage, 69, false, 0xDEADBEEF, 4);
  soft_timer_start_seconds(1, periodic_tx, NULL, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  Mcp2515Settings mcp_2515_spi_settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .int_pin = { .port = GPIO_PORT_A, 8 },

    .filters = {
      [MCP2515_FILTER_ID_RXF0] = { .raw = 0x12345678 },
      [MCP2515_FILTER_ID_RXF1] = { .raw = 0 },
    },

    .can_bitrate = MCP2515_BITRATE_125KBPS,
    .loopback = false,
  };

  LOG_DEBUG("INITIALIZING_MCP2515\n");
  StatusCode s = mcp2515_init(&s_mcp_storage, &mcp_2515_spi_settings);
  if (s) {
    LOG_DEBUG("ERROR: status code not ok: %d\n", s);
  }
  soft_timer_start_seconds(1, periodic_tx, NULL, NULL);
  mcp2515_register_cbs(&s_mcp_storage, rx_message_callback, NULL, NULL);
  while (true) {
    mcp2515_poll(&s_mcp_storage);
    //delay_ms(1000);
  }

  return 0;
}
