#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "delay.h"
#include "digital_input.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio_mcu.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

int main() {
  // Initialize all modules
  gpio_init();
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();

  // Initialize an event
  Event e = { .id = 0, .data = 0 };

  // Initialize the steering_digital_input to register
  // all interrupts and GPIO pins so they can send
  // CAN messages
  steering_digital_input_init();

  GpioAddress pin_horn = { .port = GPIO_PORT_B, .pin = 1 };
  gpio_it_trigger_interrupt(&pin_horn);
  event_process(&e);
  printf("    %d     ", e.id);

  return 0;
}
