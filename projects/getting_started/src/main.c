// Turns the LED on and sits forever
#include <stdbool.h>
#include "gpio.h"
#include "log.h"

int main(void) {
  LOG_DEBUG("Hello World!\n");

  // Init GPIO module
  gpio_init();

  // Set up PC9 as an output, default to output 0 (GND)
  GpioAddress led = {
    .port = GPIO_PORT_B,  //
    .pin = 5,             //
  };
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,  //
    .state = GPIO_STATE_HIGH,   //
  };
  gpio_init_pin(&led, &gpio_settings);

  // Add infinite loop so we don't exit
  while (true) {
  }

  return 0;
}
