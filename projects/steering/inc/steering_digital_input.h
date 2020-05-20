#pragma once
// Module to sets up all the interrupts for the GPIO pins to raise events
// in the event queue when triggered
#include "event_queue.h"
#include "gpio.h"
#include "status.h"

typedef enum {
  STEERING_DIGITAL_INPUT_HORN = 0,
  STEERING_DIGITAL_INPUT_RADIO_PPT,
  STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD,
  STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR,
  STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_TOGGLE,
  NUM_STEERING_DIGITAL_INPUTS,
} SteeringInterfaceDigitalInput;

StatusCode steering_digital_input_init();

GpioAddress *test_get_address(int digital_input_id);
