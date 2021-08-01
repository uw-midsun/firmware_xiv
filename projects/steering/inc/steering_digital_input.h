#pragma once
// Module to sets up all the interrupts for the GPIO pins to raise events
// in the event queue when triggered
#include "event_queue.h"
#include "gpio.h"
#include "status.h"

#define HORN_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 4 }
#define RADIO_PPT_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 1 }
#define DRL_1_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 6 }
#define DRL_2_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 5 }
#define REGEN_BRAKE_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 7 }
#define CC_TOGGLE_GPIO_ADDR \
  { .port = GPIO_PORT_B, .pin = 0 }
#define CC_INCREASE_SPEED_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 2 }
#define CC_DECREASE_SPEED_GPIO_ADDR \
  { .port = GPIO_PORT_A, .pin = 8 }

typedef enum {
  STEERING_DIGITAL_INPUT_HORN = 0,
  STEERING_DIGITAL_INPUT_RADIO_PPT,
  STEERING_DIGITAL_INPUT_DRL_1,
  STEERING_DIGITAL_INPUT_DRL_2,
  STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_TOGGLE,
  STEERING_DIGITAL_INPUT_CC_INCREASE_SPEED,
  STEERING_DIGITAL_INPUT_CC_DECREASE_SPEED,
  NUM_STEERING_DIGITAL_INPUTS,
} SteeringInterfaceDigitalInput;

StatusCode steering_digital_input_init();
