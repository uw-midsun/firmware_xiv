#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_transmit.h"
#include "delay.h"
#include "digital_input.h"
#include "digital_input_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

int main() {
  // Initialize all modules
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  // Set up settings for CAN
  CanStorage storage = { 0 };
  const CanSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_STEERING,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = STEERING_DIGITAL_INPUT_CAN_RX,
    .tx_event = STEERING_DIGITAL_INPUT_CAN_TX,
  };
  // Initialize CAN
  can_init(&storage, &settings);

  SteeringDigitalInputCanEvents steering[NUM_STEERING_DIGITAL_INPUT_IDS] = {
   [STEERING_DIGITAL_INPUT_EVENT_HORN] = {
            .can_event = { [GPIO_STATE_HIGH] = EE_STEERING_INPUT_HORN_PRESSED,  //
                           [GPIO_STATE_LOW] = EE_STEERING_INPUT_HORN_RELEASED 
        } 
      }  //initialize the rest
    };

  // Initialize an event
  Event e = { .id = 0, .data = 0 };

  // Initialize the steering_digital_input to register
  // all interrupts and GPIO pins so messages
  // can be sent to CAN
  steering_digital_input_init(&steering);

  while (true) {
    // Pops events off of the queue if there is an item in the queue
    // and sends a CAN message
    if (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }
  return 0;
}