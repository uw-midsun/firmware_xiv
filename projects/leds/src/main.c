// Example program for STM32F072 Controller board or Discovery Board.
// Blinks the LEDs sequentially.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "uart.h"

#include "delay.h"       // For real-time delays
#include "gpio.h"        // General Purpose I/O control.
#include "interrupt.h"   // For enabling interrupts.
#include "misc.h"        // Various helper functions/macros.
#include "soft_timer.h"  // Software timers for scheduling future events.

// Depending on which board you are working with you will need to (un)comment
// the relevant block of GPIO pins. Generally these would be in a configuration
// file somewhere and we normally write configurations only for the controller
// board.

static size_t s_len;
static uint8_t data[UART_MAX_BUFFER_LEN];
static bool updated;

// Controller board LEDs
static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};
static void rx_hnd(const uint8_t *rx_arr, size_t len, void *context) {
  s_len = len;
  memcpy(data, rx_arr, len);
  gpio_toggle_state(&leds[2]);
  updated = true;
}

static UartStorage store;
static const  UartSettings set = {
    .tx = (GpioAddress){ .port = GPIO_PORT_B, .pin = 6 },   //
    .rx = (GpioAddress){ .port = GPIO_PORT_B, .pin = 7 },   //
    .alt_fn = GPIO_ALTFN_0,
    .baudrate = 9600,
    .rx_handler = rx_hnd,
  };

// Discovery board LEDs
// static const GpioAddress leds[] = {
//  { .port = GPIO_PORT_C, .pin = 8 },  //
//  { .port = GPIO_PORT_C, .pin = 9 },  //
//  { .port = GPIO_PORT_C, .pin = 6 },  //
//  { .port = GPIO_PORT_C, .pin = 7 },  //
//};

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,        // The pin needs to output.
    .state = GPIO_STATE_LOW,         // Start in the "on" state.
    .alt_function = GPIO_ALTFN_NONE,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };

  // Init all of the LED pins
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  uart_init(UART_PORT_1, &set, &store);
  uart_set_rx_handler(UART_PORT_1, rx_hnd, NULL);
  const char *m = "Absi\n";
  size_t st = 6;
  uart_tx(UART_PORT_1, m, 6);
  // Keep toggling the state of the pins from on to off with a 50 ms delay between.
  while (true) {
    for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      if (updated) {
        uart_tx(UART_PORT_1, data, s_len);
        updated = false;
        s_len = 0;
      }
      
      //gpio_toggle_state(&leds[i]);
      delay_ms(1000);
    }
  }

  return 0;
}
