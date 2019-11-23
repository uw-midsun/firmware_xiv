#include "digital_input_config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_transmit.h"
#include "delay.h"
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
static const GpioAddress s_steering_digital_input[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = { .port = GPIO_PORT_B, .pin = 0 },
  [STEERING_DIGITAL_INPUT_RADIO_PPT] = { .port = GPIO_PORT_B, .pin = 1 },
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] = { .port = GPIO_PORT_B, .pin = 2 },
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = { .port = GPIO_PORT_B, .pin = 3 },
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = { .port = GPIO_PORT_B, .pin = 4 },
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = { .port = GPIO_PORT_A, .pin = 0 },
  [STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED] = { .port = GPIO_PORT_A, .pin = 1 },
  [STEERING_DIGITAL_DIGITAL_INPUT_CC_DECREASE_SPEED] = { .port = GPIO_PORT_A, .pin = 2 }
};

void prv_callback_raise_event(const GpioAddress *address, void *context) {
  GpioState state;
  gpio_get_state(address, &state);
  EventId *event = context;

  // Raises an event and pushes it onto the queue
  event_raise(*event, 0);
}

StatusCode steering_digital_input_init(SteeringDigitalInputConfiguration *storage) {
  GpioSettings digital_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
    gpio_init_pin(&s_steering_digital_input[i], &digital_input_settings);

    InterruptSettings interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                             .priority = INTERRUPT_PRIORITY_NORMAL };

    gpio_it_register_interrupt(&s_steering_digital_input[i], &interrupt_settings,
                               INTERRUPT_EDGE_RISING_FALLING, prv_callback_raise_event,
                               &storage[i].event);
  }

  return STATUS_CODE_OK;
}
