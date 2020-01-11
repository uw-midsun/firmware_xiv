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
  STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED,
  STEERING_DIGITAL_DIGITAL_INPUT_CC_DECREASE_SPEED,
  STEERING_DIGITAL_INPUT_CAN_TX,
  STEERING_DIGITAL_INPUT_CAN_RX,
  NUM_STEERING_DIGITAL_INPUTS,
} SteeringInterfaceDigitalInput;

// Digital Inputs
typedef struct {
  // Event that will be raised
  EventId event;
  // Pin for the input
  GpioAddress address;
} SteeringDigitalInputConfiguration;

StatusCode steering_digital_input_init(SteeringDigitalInputConfiguration *storage);
