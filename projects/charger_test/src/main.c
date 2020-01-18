// Example program for STM32F072 Controller board or Discovery Board.
// Blinks the LEDs sequentially.
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

// Controller board LEDs
GpioAddress connection_address = { .port = GPIO_PORT_B, .pin = 1 };
GpioAddress receive_interrupt_address = { .port = GPIO_PORT_A, .pin = 8 };

static void prv_receive_interrupt(const GpioAddress *address, void *context) {
  LOG_DEBUG("A message is ready!\n");
}

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  LOG_DEBUG("Hello\n");

  GpioSettings connection_settings = {
    .direction = GPIO_DIR_IN,        // The pin needs to output.
    .alt_function = GPIO_ALTFN_NONE,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  gpio_init_pin(&connection_address, &connection_settings);

  // Initialize receive interrupt
  gpio_it_register_interrupt(&receive_interrupt_address, &interrupt_settings,
                             INTERRUPT_EDGE_RISING_FALLING, prv_receive_interrupt,
                             NULL);

  // Keep toggling the state of the pins from on to off with a 50 ms delay between.
  while (true) {
    GpioState state = NUM_GPIO_STATES;
    gpio_get_state(&connection_address, &state);
    LOG_DEBUG("GPIO_STATE: %s\n", state? "high": "low");
    delay_ms(300);
  }

  return 0;
}
