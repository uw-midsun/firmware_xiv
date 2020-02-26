#include "button_press.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  button_press_init();

  LOG_DEBUG("Hello from Centre Console!\n");

  while (true) {

  }

  return 0;
}
