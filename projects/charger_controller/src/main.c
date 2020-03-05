#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
// include all the modules
#include "charger_controller.h"

// CanSettings can_settings = {
//     .device_id = TEST_CAN_DEVICE_ID,
//     .bitrate = CAN_HW_BITRATE_500KBPS,
//     .rx_event = TEST_CAN_EVENT_RX,
//     .tx_event = TEST_CAN_EVENT_TX,
//     .fault_event = TEST_CAN_EVENT_FAULT,
//     .tx = { GPIO_PORT_A, 12 },
//     .rx = { GPIO_PORT_A, 11 },
//     .loopback = false,
//   };

static Mcp2515Storage mcp2515;
static Mcp2515Settings mcp2515_settings = {
  .spi_port = SPI_PORT_2,
  .spi_baudrate = MCP2515_BITRATE_250KBPS,
  .mosi = { .port = GPIO_PORT_A, 7 },
  .miso = { .port = GPIO_PORT_A, 6 },
  .sclk = { .port = GPIO_PORT_A, 5 },
  .cs = { .port = GPIO_PORT_A, 4 },
  .int_pin = { .port = GPIO_PORT_A, 3 },

  .can_bitrate = MCP2515_BITRATE_250KBPS,
  .loopback = false,
  .context = NULL,
};

int main() {
  LOG_DEBUG("WORKING\n");
  // initialize all the modules
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  LOG_DEBUG("Initialized modules\n");

  const GpioSettings settings = {
    .state = GPIO_STATE_HIGH,
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  GpioAddress address = {
    .port = GPIO_PORT_A,
    .pin = 2
  };
  
  status_ok_or_return(gpio_init_pin(&address, &settings));

  // setup mcp2515 settings
  mcp2515_init(&mcp2515, &mcp2515_settings);
  charger_controller_init(&mcp2515);

  LOG_DEBUG("STARTING...\n");
  Event e = { 0 };
  while (true) {
    event_process(&e);
    can_process_event(&e);
  }
  return 0;
}
