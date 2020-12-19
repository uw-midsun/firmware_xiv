#include <stdbool.h>
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "max6643_fan_controller.h"
#include "wait.h"

#define FANFAIL_PIN \
  { .port = GPIO_PORT_B, .pin = 4 }
#define OVERTEMP_PIN \
  { .port = GPIO_PORT_B, .pin = 5 }
#define FAN_PIN \
  { .port = GPIO_PORT_B, .pin = 3 }
#define FAN_SPEED_HIGH true

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
  GpioAddress fanfail_pin_address = FANFAIL_PIN;
  GpioAddress overtemp_pin_address = OVERTEMP_PIN;
  GpioAddress fan_pin_address = FAN_PIN;
  bool fan_speed_toggle = FAN_SPEED_HIGH;
  GpioSettings gpio_high_settings = {
    .direction = GPIO_DIR_OUT,  //
    .state = GPIO_STATE_HIGH,   //
  };
  GpioSettings gpio_low_settings = {
    .direction = GPIO_DIR_OUT,  //
    .state = GPIO_STATE_LOW,    //
  };
  if (fan_speed_toggle) {
    gpio_init_pin(&fan_pin_address, &gpio_high_settings);
    gpio_set_state(&fan_pin_address, GPIO_STATE_HIGH);
  } else {
    gpio_init_pin(&fan_pin_address, &gpio_low_settings);
    gpio_set_state(&fan_pin_address, GPIO_STATE_LOW);
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
