#pragma once
// Module for recieving digital inputs from steering interface (GPIO)
// Requires GPIO,Interrupts,Event Queue,Soft-timer
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"

// Digital inputs IDs
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
