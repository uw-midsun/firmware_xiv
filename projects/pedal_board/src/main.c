#include "ads1015.h"
#include "can.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

#include "drive_fsm.h"
#include "throttle.h"

static Fsm fsm;
static Ads1015Storage ads1015_storage;
static ThrottleStorage throttle_storage;

int main() {
  // initialize all the modules
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  LOG_DEBUG("Initialized modules\n");

  // Initialize FSMs
  drive_fsm_init(&fsm, &throttle_storage);

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 5 },
    .sda = { .port = GPIO_PORT_B, .pin = 5 },  // Need to change later
  };

  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };

  // Intialize ADS1015
  ads1015_init(&ads1015_storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  // Initialize throttle module
  throttle_init(&throttle_storage);

  Event e = { 0 };
  while (true) {
    event_process(&e);

    fsm_process_event(&fsm, &e);
  }
  return 0;
}
