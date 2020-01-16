#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_transmit.h"
#include "delay.h"
#include "digital_input.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"
#include "log.h"

int main() {
  // Initialize all modules
  gpio_init();
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  // Set up settings for CAN
  static CanStorage storage = { 0 };
  const CanSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_STEERING,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = STEERING_DIGITAL_INPUT_CAN_RX,
    .tx_event = STEERING_DIGITAL_INPUT_CAN_TX,
    .tx = { GPIO_PORT_A, 11 },
    .rx = { GPIO_PORT_A, 12 },
  };
 
  can_init(&storage,&settings);

  // Initialize an event
  Event e = { .id = 0, .data = 0 };

  // Initialize the steering_digital_input to register
  // all interrupts and GPIO pins so they can send
  // CAN messages
  steering_digital_input_init();
  while (true) {
    if (status_ok(event_process(&e))) {
    can_process_event(&e);
    }
  }
  return 0;
}
