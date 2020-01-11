#include "digital_input_config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_transmit.h"
#include "delay.h"
#include "digital_input_events.h"
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
DigitalInputToEventMapping s_steering_event_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = { .event = STEERING_DIGITAL_INPUT_EVENT_HORN,
                                    .address = { .port = GPIO_PORT_B, .pin = 0 } },
  [STEERING_DIGITAL_INPUT_RADIO_PPT] = { .event = STEERING_DIGITAL_INPUT_EVENT_RADIO_PPT,
                                         .address = { .port = GPIO_PORT_B, .pin = 1 } },
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] = { .event =
                                                     STEERING_DIGITAL_INPUT_EVENT_HIGH_BEAM_FORWARD,
                                                 .address = { .port = GPIO_PORT_B, .pin = 3 } },
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = { .event = STEERING_DIGITAL_INPUT_EVENT_HIGH_BEAM_REAR,
                                              .address = { .port = GPIO_PORT_B, .pin = 4 } },
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] =
      { .event = STEERING_DIGITAL_INPUT_EVENT_REGEN_BRAKE_TOGGLE,
        .address = { .port = GPIO_PORT_B, .pin = 5 } },
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = { .event = STEERING_DIGITAL_INPUT_EVENT_CC_TOGGLE,
                                         .address = { .port = GPIO_PORT_B, .pin = 6 } },
  [STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED] =
      { .event = STEERING_DIGITAL_DIGITAL_INPUT_EVENT_CC_INCREASE_SPEED,
        .address = { .port = GPIO_PORT_B, .pin = 7 } },
  [STEERING_DIGITAL_DIGITAL_INPUT_CC_DECREASE_SPEED] =
      { .event = STEERING_DIGITAL_DIGITAL_INPUT_EVENT_CC_DECREASE_SPEED,
        .address = { .port = GPIO_PORT_B, .pin = 8 } },
};

void prv_callback_raise_event(const GpioAddress *address, void *context) {
  GpioState state;
  gpio_get_state(address, &state);
  EventId *event = context;
  // Raises an event and pushes it onto the queue
  event_raise(*event, 0);
}

StatusCode steering_digital_input_init() {
  GpioSettings digital_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
    gpio_init_pin(&s_steering_event_lookup_table[i].address, &digital_input_settings);

    InterruptSettings interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                             .priority = INTERRUPT_PRIORITY_NORMAL };

    gpio_it_register_interrupt(&s_steering_event_lookup_table[i].address, &interrupt_settings,
                               INTERRUPT_EDGE_RISING_FALLING, prv_callback_raise_event,
                               &s_steering_event_lookup_table[i].event);
  }
  return STATUS_CODE_OK;
}
