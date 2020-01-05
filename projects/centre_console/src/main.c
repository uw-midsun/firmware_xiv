#include "log.h"
#include "gpio.h"
#include "gpio_it.h"
#include "delay.h"
#include "interrupt.h"
#include "event_queue.h"
#include "soft_timer.h"
#include "button_press.h"


int main(void)  {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  button_press_init();
  LOG_DEBUG("Hello from Centre Console!\n");

  GpioAddress *addresses = test_provide_button_addresses();
  GpioAddress power_button = addresses[CENTRE_CONSOLE_BUTTON_POWER];
  while (true) {
    gpio_it_trigger_interrupt(&power_button);
    delay_ms(400);
  }

  return 0;
}
