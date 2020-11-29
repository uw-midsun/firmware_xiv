#include <stdbool.h>

#include "delay.h"
#include "drv120_relay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// Toggle the following to select which data is logged
#define ENABLE_PIN \
  { .port = GPIO_PORT_A, .pin = 8 }
#define STATUS_PIN \
  { .port = GPIO_PORT_A, .pin = 6 }
#define TIME_INTERVAL 500

static void prv_error_handler(void *context) {
  LOG_DEBUG("Error %s\n", "error occured");
}

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  bool *toggle = context;

  if (*toggle) {
    drv120_relay_open();
    *toggle = false;

  } else {
    drv120_relay_close();
    *toggle = true;
  }

  soft_timer_start_millis(TIME_INTERVAL, prv_timer_callback, toggle, NULL);
}
int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_it_init();
  gpio_init();
  GpioAddress status_pin_address = STATUS_PIN;
  GpioAddress enable_pin_address = ENABLE_PIN;
  bool toggle = true;

  Drv120RelaySettings relay_settings = { .enable_pin = &enable_pin_address,
                                         .status_pin = &status_pin_address,
                                         .error_handler = prv_error_handler,
                                         NULL };

  drv120_relay_init(&relay_settings);
  soft_timer_start_millis(TIME_INTERVAL, prv_timer_callback, &toggle, NULL);

  while (true) {
    wait();
  }
}
