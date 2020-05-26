#pragma once
// Module to sets up all the interrupts for the GPIO pins to raise events
// in the event queue when triggered
#include "event_queue.h"
#include "gpio.h"
#include "status.h"

#define HORN_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 1 }
#define RADIO_PPT_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 6 }
#define HIGH_BEAM_FORWARD_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 7 }
#define HIGH_BEAM_REAR_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 0 }
#define REGEN_BRAKE_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 4 }
#define CC_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 5 }

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
