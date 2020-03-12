#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "mcp2515.h"
// include all the modules

int main(void) {
  LOG_DEBUG("Welcome to Charger!\n");
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
  GpioAddress address = { .port = GPIO_PORT_A, .pin = 2 };

  status_ok_or_return(gpio_init_pin(&address, &settings));
 
    return 0;
}