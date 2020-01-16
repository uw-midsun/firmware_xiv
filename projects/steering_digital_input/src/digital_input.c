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
  [STEERING_DIGITAL_INPUT_HORN] = { .event = {
          [GPIO_STATE_LOW] = EE_STEERING_INPUT_HORN_PRESSED,
          [GPIO_STATE_HIGH] = EE_STEERING_INPUT_HORN_RELEASED,
  },
          .address = { .port = GPIO_PORT_B, .pin = 0 } },

    [STEERING_DIGITAL_INPUT_RADIO_PPT] = { .event = {
          [GPIO_STATE_LOW] = EE_STEERING_RADIO_PPT_PRESSED,
          [GPIO_STATE_HIGH] = EE_STEERING_RADIO_PPT_RELEASED
  },
            .address = { .port = GPIO_PORT_B, .pin = 1 } },
    [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] =
        { .event = {
          [GPIO_STATE_LOW] = EE_STEERING_HIGH_BEAM_FORWARD_ON,
          [GPIO_STATE_HIGH] =  EE_STEERING_HIGH_BEAM_FORWARD_OFF
  }, 
          .address = { .port = GPIO_PORT_B, .pin = 3 } },
    [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = { .event = {
          [GPIO_STATE_LOW] =  EE_STEERING_HIGH_BEAM_REAR_ON,
          [GPIO_STATE_HIGH] =  EE_STEERING_HIGH_BEAM_REAR_OFF
  },
             .address = { .port = GPIO_PORT_B, .pin = 4 } },
    [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] =
        { .event = {
          [GPIO_STATE_LOW] = EE_STEERING_REGEN_BRAKE_ON,
          [GPIO_STATE_HIGH] = EE_STEERING_REGEN_BRAKE_OFF
  },
          .address = { .port = GPIO_PORT_B, .pin = 5 } },
    [STEERING_DIGITAL_INPUT_CC_TOGGLE] = { .event = {
          [GPIO_STATE_LOW] = EE_STEERING_INPUT_CC_TOGGLE_PRESSED,
  },
         .address = { .port = GPIO_PORT_B, .pin = 6 } },
    [STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED] =
        { .event = {
          [GPIO_STATE_LOW] =  EE_STEERING_INPUT_CC_SPEED_PLUS_PRESSED,
  },
          .address = { .port = GPIO_PORT_B, .pin = 7 } },
    [STEERING_DIGITAL_DIGITAL_INPUT_CC_DECREASE_SPEED] =
        { .event = {
          [GPIO_STATE_LOW] = EE_STEERING_INPUT_CC_SPEED_MINUS_PRESSED,
  },
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
  
  LOG_DEBUG("HELLLOOO"); //This does not seem to be working




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
