#include "steering_digital_input.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "interrupt_def.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_can.h"
#include "steering_events.h"

GpioAddress s_steering_address_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = { .port = GPIO_PORT_B, .pin = 1 },
  [STEERING_DIGITAL_INPUT_RADIO_PPT] = { .port = GPIO_PORT_A, .pin = 6 },
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] = { .port = GPIO_PORT_A, .pin = 7 },
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = { .port = GPIO_PORT_B, .pin = 0 },
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = { .port = GPIO_PORT_A, .pin = 4 },
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = { .port = GPIO_PORT_A, .pin = 5 },
};

EventId s_steering_event_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
  [STEERING_DIGITAL_INPUT_HORN] = STEERING_INPUT_HORN_EVENT,
  [STEERING_DIGITAL_INPUT_RADIO_PPT] = STEERING_RADIO_PPT_EVENT,
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] = STEERING_HIGH_BEAM_FORWARD_EVENT,
  [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = STEERING_HIGH_BEAM_REAR_EVENT,
  [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] = STEERING_REGEN_BRAKE_EVENT,
  [STEERING_DIGITAL_INPUT_CC_TOGGLE] = STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT
};

GpioAddress *test_get_address(int digital_input_id) {
  return &s_steering_address_lookup_table[digital_input_id];
}

void prv_callback_raise_event(const GpioAddress *address, void *context) {
  GpioState state;
  gpio_get_state(address, &state);
  EventId *event = context;

  event_raise(*event, state);
}

StatusCode steering_digital_input_init() {
  GpioSettings digital_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  for (int i = 0; i < NUM_STEERING_DIGITAL_INPUTS; i++) {
    gpio_init_pin(&s_steering_address_lookup_table[i], &digital_input_settings);

    InterruptSettings interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                             .priority = INTERRUPT_PRIORITY_NORMAL };

    if (i == STEERING_DIGITAL_INPUT_HORN || i == STEERING_DIGITAL_INPUT_RADIO_PPT) {
      gpio_it_register_interrupt(&s_steering_address_lookup_table[i], &interrupt_settings,
                                 INTERRUPT_EDGE_RISING_FALLING, prv_callback_raise_event,
                                 &s_steering_event_lookup_table[i]);
    } else {
      gpio_it_register_interrupt(&s_steering_address_lookup_table[i], &interrupt_settings,
                                 INTERRUPT_EDGE_FALLING, prv_callback_raise_event,
                                 &s_steering_event_lookup_table[i]);
    }
  }
  return STATUS_CODE_OK;
}
