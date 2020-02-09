#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
// include all the modules
#include "charger_controller.h"

static Mcp2515Storage mcp2515;
const Mcp2515Settings mcp2515_settings = {
  .spi_port = SPI_PORT_1,
  .spi_baudrate = 750000,
  .mosi = { .port = GPIO_PORT_A, 7 },
  .miso = { .port = GPIO_PORT_A, 6 },
  .sclk = { .port = GPIO_PORT_A, 5 },
  .cs = { .port = GPIO_PORT_A, 4 },
  .int_pin = { .port = GPIO_PORT_A, 3 },

  .loopback = false,
  .context = NULL,
};
ChargerData data = {
  .storage = &mcp2515,
  .id = 0,
  .extended = false,
  .data = 0,
  .dlc = 0,
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

  // setup mcp2515 settings
  mcp2515_init(&mcp2515, &mcp2515_settings);
  charger_controller_init(&data);

  Event e = { 0 };
  while (true) {
    event_process(&e);
    can_process_event(&e);
  }
  return 0;
}
