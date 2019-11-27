#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_transmit.h"
#include "delay.h"
#include "digital_input_config.h"
#include "digital_input_events.h"
#include "steering_can.h"
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
  };
  // Initialize CAN
  can_init(&storage, &settings);
  steering_can_init();

  static SteeringDigitalInputConfiguration s_steering_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
    [STEERING_DIGITAL_INPUT_HORN] = 
    { .event = STEERING_DIGITAL_INPUT_EVENT_HORN,
      .address = { .port = GPIO_PORT_B, .pin = 0 } },
    [STEERING_DIGITAL_INPUT_RADIO_PPT] = 
    { .event = STEERING_DIGITAL_INPUT_EVENT_RADIO_PPT,
      .address = { .port = GPIO_PORT_B, .pin = 1 } },
    [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] =
        { .event = STEERING_DIGITAL_INPUT_EVENT_HIGH_BEAM_FORWARD,
          .address = { .port = GPIO_PORT_B, .pin = 3 } },
    [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = 
       { .event =  STEERING_DIGITAL_INPUT_EVENT_HIGH_BEAM_REAR,
         .address = { .port = GPIO_PORT_B, .pin = 4 } },
    [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] =
        { .event = STEERING_DIGITAL_INPUT_EVENT_REGEN_BRAKE_TOGGLE,
          .address = { .port = GPIO_PORT_B, .pin = 5 } },
    [STEERING_DIGITAL_INPUT_CC_TOGGLE] = 
        { .event = STEERING_DIGITAL_INPUT_EVENT_CC_TOGGLE,
          .address = { .port = GPIO_PORT_B, .pin = 6 } },
    [STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED] =
        { .event = STEERING_DIGITAL_DIGITAL_INPUT_EVENT_CC_INCREASE_SPEED,
          .address = { .port = GPIO_PORT_B, .pin = 7 } },
    [STEERING_DIGITAL_DIGITAL_INPUT_CC_DECREASE_SPEED] =
        { .event = STEERING_DIGITAL_DIGITAL_INPUT_EVENT_CC_DECREASE_SPEED,
          .address = { .port = GPIO_PORT_B, .pin = 8 } },
  };

  // Initialize an event
  Event e = { .id = 0, .data = 0 };
    
  // Initialize the steering_digital_input to register
  // all interrupts and GPIO pins so messages
  // can be sent to CAN
  steering_digital_input_init(s_steering_lookup_table);

  while (true) {
    // Pops events off of the queue if there is an item in the queue
    // and sends a CAN message
   steering_can_process_event(&e);
  }
  return 0;
}
