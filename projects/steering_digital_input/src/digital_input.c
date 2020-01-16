#include "digital_input.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "log.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

// Set up all GPIO Addresses to each button
// that recieves digital inputs
DigitalInputToEventMapping s_steering_event_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = { 
  [STEERING_DIGITAL_INPUT_HORN] = { .event = STEERING_DIGITAL_INPUT_HORN,
                                    .address = { .port = GPIO_PORT_B, .pin = 0 } },
    [STEERING_DIGITAL_INPUT_RADIO_PPT] = { .event = STEERING_DIGITAL_INPUT_RADIO_PPT,
                                           .address = { .port = GPIO_PORT_B, .pin = 1 } },
    [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] =
        { .event = STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD,
          .address = { .port = GPIO_PORT_B, .pin = 3 } },
    [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = { .event =
                                                    STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR,
                                                .address = { .port = GPIO_PORT_B, .pin = 4 } },
    [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] =
        { .event = STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE,
          .address = { .port = GPIO_PORT_B, .pin = 5 } },
    [STEERING_DIGITAL_INPUT_CC_TOGGLE] = { .event = STEERING_DIGITAL_INPUT_CC_TOGGLE,
                                           .address = { .port = GPIO_PORT_B, .pin = 6 } },
    [STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED] =
        { .event = STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED,
          .address = { .port = GPIO_PORT_B, .pin = 7 } },
    [STEERING_DIGITAL_DIGITAL_INPUT_CC_DECREASE_SPEED] =
        { .event = STEERING_DIGITAL_DIGITAL_INPUT_CC_DECREASE_SPEED,
          .address = { .port = GPIO_PORT_B, .pin = 8 } }
  };

void prv_callback_raise_event(const GpioAddress *address, void *context) {
  GpioState state;
  gpio_get_state(address, &state);
  EventId *can_event = context;
  // Raises an event and pushes it onto the queue to be sent with a CAN message
  CAN_TRANSMIT_STEERING_EVENT(can_event[(size_t)state], 0);

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

    // Only the interrupt for the horn and radio should
    // be triggered by rising and falling and everything else
    // is triggered when falling
    if (i == STEERING_DIGITAL_INPUT_HORN || i == STEERING_DIGITAL_INPUT_RADIO_PPT) {
      gpio_it_register_interrupt(&s_steering_event_lookup_table[i].address, &interrupt_settings,
                                 INTERRUPT_EDGE_RISING_FALLING, prv_callback_raise_event,
                                 &s_steering_event_lookup_table[i].event);
    } else {
      gpio_it_register_interrupt(&s_steering_event_lookup_table[i].address, &interrupt_settings,
                                 INTERRUPT_EDGE_FALLING, prv_callback_raise_event,
                                 &s_steering_event_lookup_table[i].event);
    }
  }
  return STATUS_CODE_OK;
}
