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

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  Mcp2515Settings mcp_2515_spi_settings = {
    .spi_port = SPI_PORT_2,
    .baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, .pin = 15},
    .miso = { .port = GPIO_PORT_B, .pin = 14},
    .sclk = { .port = GPIO_PORT_B, .pin = 13},
    .cs = { .port = GPIO_PORT_B, .pin = 12},
    .int_pin = { .port = GPIO_PORT_A, .pin = 8},
    .loopback = false,
    .rx_cb = rx_message_callback
  };

  LOG_DEBUG("INITIALIZING_MCP2515\n");
  StatusCode s = mcp2515_init(&s_mcp_storage, &mcp_2515_spi_settings);
  if (s) {
    LOG_DEBUG("ERROR: status code not ok: %d\n", s);
  }
  mcp2515_register_rx_cb(&s_mcp_storage, rx_message_callback, NULL);
  while (true) {
    mcp2515_tx(&s_mcp_storage, 69, false, 0xDEADBEEF, 4);
    delay_ms(1000);
  }

  return 0;
}
