#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_transmit.h"
#include "delay.h"
#include "digital_input.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

// Set up all GPIO Addresses to each button
// that recieves digital inputs
static const GpioAddress steering_digital_input[NUM_STEERING_DIGITAL_INPUT_IDS] = {
  [STEERING_DIGITAL_INPUT_ID_HORN] = { .port = GPIO_PORT_B, .pin = 0 },
  [STEERING_DIGITAL_INPUT_ID_RADIO_PPT] = { .port = GPIO_PORT_B, .pin = 1 },
  [STEERING_DIGITAL_INPUT_ID_HIGH_BEAM_FWD] = { .port = GPIO_PORT_B, .pin = 2 },
  [STEERING_DIGITAL_INPUT_ID_HIGH_BEAM_BACK] = { .port = GPIO_PORT_B, .pin = 3 },
  [STEERING_DIGITAL_INPUT_ID_REGEN_BRAKE_TOGGLE] = { .port = GPIO_PORT_B, .pin = 4 },
  [STEERING_DIGITAL_INPUT_ID_HEADLIGHTS_ON_OFF] = { .port = GPIO_PORT_B, .pin = 5 },
  [STEERING_DIGITAL_INPUT_ID_CC_ON_OFF] = { .port = GPIO_PORT_A, .pin = 0 },
  [STEERING_DIGITAL_DIGITAL_INPUT_ID_CC_INCREASE_SPEED] = { .port = GPIO_PORT_A, .pin = 1 },
  [STEERING_DIGITAL_DIGITAL_INPUT_ID_Cc_DECREASE_SPEED] = { .port = GPIO_PORT_A, .pin = 2 }
};

// Raises an event
void prv_callback_raise_event(const GpioAddress *address, void *context) {
  // Sends the index as the id and the data to the event(which is defined in the
  // enum in the digital_input_event file)

  EventId *id = context;
  uint16_t *data = context;

  event_raise_priority(EVENT_PRIORITY_NORMAL, *id, *data);
}

StatusCode steering_digital_input_init() {
  // Initialize all GPIO pins for Digital Inputs
  GpioSettings digital_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  // Go through each digital input pin and initialize them
  for (int i = 0; i < NUM_STEERING_DIGITAL_INPUT_IDS; i++) {
    gpio_init_pin(&steering_digital_input[i], &digital_input_settings);

    // Set up interrupts for each digital input pin
    InterruptSettings interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                             .priority = INTERRUPT_PRIORITY_NORMAL };

    // Registers interrupts for each digital input pin
    gpio_it_register_interrupt(&steering_digital_input[i], &interrupt_settings,
                               INTERRUPT_EDGE_RISING_FALLING, prv_callback_raise_event, &i);
  }

  return STATUS_CODE_OK;
}

int main() {
  // Initialize all modules
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  // Initialize an event
  Event e = { .id = 0, .data = 0 };

  while (true) {

    // Pops events off of the queue if there is an item in the queue
    if (event_process(&e) != STATUS_CODE_EMPTY) {
      CAN_TRANSMIT_STEERING_EVENT(e.id, e.data);
    }
  }
}
