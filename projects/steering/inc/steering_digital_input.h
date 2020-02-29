#pragma once
// Module to sets up all the interrupts for the GPIO pins to raise events
// in the event queue when triggered
// Requires GPIO,Interrupts,Event Queue,Soft-timer
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
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

typedef enum {
  STEERING_INPUT_HORN_EVENT = 0,
  STEERING_RADIO_PPT_EVENT,
  STEERING_HIGH_BEAM_FORWARD_EVENT,
  STEERING_HIGH_BEAM_REAR_EVENT,
  STEERING_REGEN_BRAKE_EVENT,
  STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT,
  NUM_STEERING_EVENTS,
} SteeringEvent;

StatusCode steering_digital_input_init();

GpioAddress *test_get_address(int digital_input_id);
