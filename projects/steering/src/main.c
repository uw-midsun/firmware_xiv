#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_digital_input.h"
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
  while (event_process(&e)) {
    // can_process_steering_event();  TO BE CREATED
  }
  return 0;
}
