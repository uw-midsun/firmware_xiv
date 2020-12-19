#include <stdbool.h>
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "max6643_fan_controller.h"
#include "wait.h"
// The smoke test (in a project called smoke_max6643) uses the MAX6643 driver
// to register two separate callbacks which LOG_WARN a warning when the fan fail or
// overtemp conditions are signalled.It also have a #define bool constant as a parameter
// if that constant is true, it should turn the full
// speed pin high, and if false, it should turn it low.

#define FAN_FULL_SPEED true

static void prv_fanfail_callback(const GpioAddress *address, void *context) {
  LOG_WARN("Error %s\n", "Warning: The fan failed");
}

static void prv_overtemp_callback(const GpioAddress *address, void *context) {
  LOG_WARN("Error %s\n", "Warning: Overtemp signal has been called");
}

int main(void) {
  interrupt_init();
  gpio_it_init();
  gpio_init();
  GpioAddress fanfail_pin_address = { .port = GPIO_PORT_B, .pin = 4 };
  GpioAddress overtemp_pin_address = { .port = GPIO_PORT_B, .pin = 5 };
  GpioAddress fan_pin_address = { .port = GPIO_PORT_B, .pin = 3 };
  bool fan_speed_toggle = FAN_FULL_SPEED;
  GpioSettings gpio_high_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };
  GpioSettings gpio_low_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };
  if (fan_speed_toggle) {
    gpio_init_pin(&fan_pin_address, &gpio_high_settings);
  } else {
    gpio_init_pin(&fan_pin_address, &gpio_low_settings);
  }
  Max6643Settings settings = { .fanfail_pin = fanfail_pin_address,
                               .overtemp_pin = overtemp_pin_address,
                               .fanfail_callback = prv_fanfail_callback,
                               .overtemp_callback = prv_overtemp_callback,
                               .fanfail_callback_context = NULL,
                               .overtemp_callback_context = NULL };
  max6643_init(&settings);
  while (true) {
    wait();
  }
}
